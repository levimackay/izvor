#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "diag.h"
#include "interp.h"

typedef struct {
    Name name;
    long value;
} Slot;

typedef struct {
    const char *src;
    const Program *prog;
    Slot *slots;
    int count;
    int capacity;
    int base;
    int depth;
    bool returning;
    long result;
} Interp;

static void fail(const Interp *in, long offset, const char *msg) {
    diag_runtime_error(in->src, offset, msg);
    exit(1);
}

static void push(Interp *in, Name name, long value) {
    if (in->count == in->capacity) {
        in->capacity = in->capacity == 0 ? 64 : in->capacity * 2;
        in->slots = xrealloc(in->slots, sizeof(Slot) * (size_t)in->capacity);
    }
    in->slots[in->count].name = name;
    in->slots[in->count].value = value;
    in->count++;
}

static Slot *lookup(Interp *in, Name name) {
    for (int i = in->count - 1; i >= in->base; i--) {
        if (name_eq(in->slots[i].name, name)) return &in->slots[i];
    }
    fprintf(stderr, "internal error: '%.*s' has no value\n", name.length, name.start);
    exit(1);
}

static const Function *find_function(const Interp *in, Name name) {
    for (const Function *fn = in->prog->functions; fn != NULL; fn = fn->next) {
        if (name_eq(fn->name, name)) return fn;
    }
    fprintf(stderr, "internal error: no function '%.*s'\n", name.length, name.start);
    exit(1);
}

static long eval(Interp *in, const Node *node);
static void exec_block(Interp *in, const Stmt *body);

static long arithmetic(const Interp *in, const Node *node, long left, long right) {
    long result = 0;
    bool overflow = false;

    switch (node->as.binary.op) {
    case TOK_PLUS:
        overflow = __builtin_add_overflow(left, right, &result);
        break;
    case TOK_MINUS:
        overflow = __builtin_sub_overflow(left, right, &result);
        break;
    case TOK_STAR:
        overflow = __builtin_mul_overflow(left, right, &result);
        break;
    case TOK_SLASH:
    case TOK_PERCENT:
        if (right == 0) fail(in, node->offset, "division by zero");
        overflow = left == LONG_MIN && right == -1;
        if (!overflow) result = node->as.binary.op == TOK_SLASH ? left / right : left % right;
        break;
    default:
        break;
    }

    if (overflow) fail(in, node->offset, "integer overflow");
    return result;
}

static long eval_binary(Interp *in, const Node *node) {
    TokenType op = node->as.binary.op;
    long left = eval(in, node->as.binary.left);

    if (op == TOK_AND && !left) return 0;
    if (op == TOK_OR && left) return 1;

    long right = eval(in, node->as.binary.right);
    switch (op) {
    case TOK_AND:
    case TOK_OR: return right != 0;
    case TOK_EQUAL_EQUAL: return left == right;
    case TOK_BANG_EQUAL: return left != right;
    case TOK_LESS: return left < right;
    case TOK_LESS_EQUAL: return left <= right;
    case TOK_GREATER: return left > right;
    case TOK_GREATER_EQUAL: return left >= right;
    default: return arithmetic(in, node, left, right);
    }
}

static long call(Interp *in, const Node *node) {
    const Function *fn = find_function(in, node->as.call.callee);
    int frame = in->count;
    char msg[64];
    Name unnamed = {"", 0};

    for (int i = 0; i < node->as.call.arg_count; i++) {
        long value = eval(in, node->as.call.args[i]);
        push(in, unnamed, value);
    }
    for (int i = 0; i < fn->param_count; i++) {
        in->slots[frame + i].name = fn->params[i].name;
    }

    if (++in->depth > MAX_CALLS) {
        snprintf(msg, sizeof msg, "more than %d nested calls", MAX_CALLS);
        fail(in, fn->offset, msg);
    }

    int saved_base = in->base;
    in->base = frame;
    in->result = 0;
    exec_block(in, fn->body);
    in->depth--;

    long result = in->result;
    in->returning = false;
    in->base = saved_base;
    in->count = frame;
    return result;
}

static long eval(Interp *in, const Node *node) {
    switch (node->type) {
    case NODE_NUMBER:
        return node->as.number;
    case NODE_BOOL:
        return node->as.boolean;
    case NODE_NAME:
        return lookup(in, node->as.name)->value;
    case NODE_UNARY: {
        long value = eval(in, node->as.unary.operand);
        if (node->as.unary.op == TOK_BANG) return !value;
        if (value == LONG_MIN) fail(in, node->offset, "integer overflow");
        return -value;
    }
    case NODE_BINARY:
        return eval_binary(in, node);
    case NODE_CALL:
        return call(in, node);
    }
    fprintf(stderr, "internal error: unknown node type\n");
    exit(1);
}

static void print_value(const Node *node, long value) {
    if (node->ty == TYPE_BOOL) {
        printf("%s\n", value ? "true" : "false");
    } else {
        printf("%ld\n", value);
    }
}

static void exec(Interp *in, const Stmt *stmt) {
    switch (stmt->type) {
    case STMT_LET:
        push(in, stmt->as.let.name, eval(in, stmt->as.let.value));
        break;
    case STMT_ASSIGN: {
        long value = eval(in, stmt->as.assign.value);
        lookup(in, stmt->as.assign.name)->value = value;
        break;
    }
    case STMT_PRINT:
        print_value(stmt->as.expr, eval(in, stmt->as.expr));
        break;
    case STMT_IF:
        if (eval(in, stmt->as.if_.cond)) {
            exec_block(in, stmt->as.if_.then_body);
        } else {
            exec_block(in, stmt->as.if_.else_body);
        }
        break;
    case STMT_WHILE:
        while (!in->returning && eval(in, stmt->as.while_.cond)) {
            exec_block(in, stmt->as.while_.body);
        }
        break;
    case STMT_RETURN:
        in->result = stmt->as.expr == NULL ? 0 : eval(in, stmt->as.expr);
        in->returning = true;
        break;
    case STMT_EXPR:
        eval(in, stmt->as.expr);
        break;
    }
}

static void exec_block(Interp *in, const Stmt *body) {
    int mark = in->count;
    for (const Stmt *stmt = body; stmt != NULL && !in->returning; stmt = stmt->next) {
        exec(in, stmt);
    }
    in->count = mark;
}

static void *run_thread(void *arg) {
    Interp *in = arg;
    exec_block(in, in->prog->statements);
    return NULL;
}

void interp_run(const char *src, const Program *prog) {
    Interp in = {src, prog, NULL, 0, 0, 0, 0, false, 0};

    pthread_attr_t attr;
    pthread_t thread;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, (size_t)512 << 20);
    if (pthread_create(&thread, &attr, run_thread, &in) == 0) {
        pthread_join(thread, NULL);
    } else {
        run_thread(&in);
    }
    pthread_attr_destroy(&attr);

    free(in.slots);
}

long interp_eval(const char *src, const Node *expr) {
    Interp in = {src, NULL, NULL, 0, 0, 0, 0, false, 0};
    long value = eval(&in, expr);
    free(in.slots);
    return value;
}
