#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "check.h"
#include "diag.h"

typedef struct {
    Name name;
    Type type;
    bool mutable;
} Local;

typedef struct {
    const char *src;
    const Program *prog;
    const Function *fn;
    Local *locals;
    int count;
    int capacity;
    int errors;
} Checker;

static void error(Checker *c, long offset, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

static void error(Checker *c, long offset, const char *fmt, ...) {
    char msg[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof msg, fmt, args);
    va_end(args);
    diag_error(c->src, offset, "%s", msg);
    c->errors++;
}

static long name_offset(const Checker *c, Name name) {
    return name.start - c->src;
}

static long start_of(const Node *node) {
    switch (node->type) {
    case NODE_BINARY:
        return start_of(node->as.binary.left);
    case NODE_NUMBER:
    case NODE_BOOL:
    case NODE_NAME:
    case NODE_UNARY:
    case NODE_CALL:
        break;
    }
    return node->offset;
}

static const Function *find_function(const Checker *c, Name name) {
    if (c->prog == NULL) return NULL;
    for (const Function *fn = c->prog->functions; fn != NULL; fn = fn->next) {
        if (name_eq(fn->name, name)) return fn;
    }
    return NULL;
}

static Local *find_local(Checker *c, Name name) {
    for (int i = c->count - 1; i >= 0; i--) {
        if (name_eq(c->locals[i].name, name)) return &c->locals[i];
    }
    return NULL;
}

static void declare(Checker *c, Name name, Type type, bool mutable) {
    long offset = name_offset(c, name);
    if (find_local(c, name) != NULL) {
        error(c, offset, "'%.*s' is already defined", name.length, name.start);
        return;
    }
    if (find_function(c, name) != NULL) {
        error(c, offset, "'%.*s' is already the name of a function", name.length, name.start);
        return;
    }
    if (c->count == c->capacity) {
        c->capacity = c->capacity == 0 ? 16 : c->capacity * 2;
        c->locals = xrealloc(c->locals, sizeof(Local) * (size_t)c->capacity);
    }
    c->locals[c->count].name = name;
    c->locals[c->count].type = type;
    c->locals[c->count].mutable = mutable;
    c->count++;
}

static Type check_expr(Checker *c, Node *node);

static Type check_value(Checker *c, Node *node) {
    Type type = check_expr(c, node);
    if (type != TYPE_VOID) return type;
    Name callee = node->as.call.callee;
    error(c, node->offset, "'%.*s' does not return a value", callee.length, callee.start);
    node->ty = TYPE_ERROR;
    return TYPE_ERROR;
}

static void expect_type(Checker *c, const Node *node, Type found, Type want, const char *what) {
    if (found == TYPE_ERROR || found == want) return;
    error(c, start_of(node), "%s must be %s, found %s", what, type_name(want), type_name(found));
}

static void check_operands(Checker *c, const Node *node, Type left, Type right, Type want) {
    if (left == TYPE_ERROR || right == TYPE_ERROR) return;
    if (left == want && right == want) return;
    Type found = left != want ? left : right;
    error(c, node->offset, "'%s' needs %s on both sides, found %s",
          op_symbol(node->as.binary.op), type_name(want), type_name(found));
}

static Type check_binary(Checker *c, Node *node) {
    Type left = check_value(c, node->as.binary.left);
    Type right = check_value(c, node->as.binary.right);

    switch (node->as.binary.op) {
    case TOK_EQUAL_EQUAL:
    case TOK_BANG_EQUAL:
        if (left != TYPE_ERROR && right != TYPE_ERROR && left != right) {
            error(c, node->offset, "cannot compare %s with %s", type_name(left), type_name(right));
        }
        return TYPE_BOOL;
    case TOK_AND:
    case TOK_OR:
        check_operands(c, node, left, right, TYPE_BOOL);
        return TYPE_BOOL;
    case TOK_LESS:
    case TOK_LESS_EQUAL:
    case TOK_GREATER:
    case TOK_GREATER_EQUAL:
        check_operands(c, node, left, right, TYPE_INT);
        return TYPE_BOOL;
    default:
        check_operands(c, node, left, right, TYPE_INT);
        return TYPE_INT;
    }
}

static Type check_call(Checker *c, Node *node) {
    Name callee = node->as.call.callee;
    int count = node->as.call.arg_count;
    Node **args = node->as.call.args;

    for (int i = 0; i < count; i++) {
        check_value(c, args[i]);
    }

    const Function *fn = find_function(c, callee);
    if (fn == NULL) {
        error(c, node->offset, "undefined function '%.*s'", callee.length, callee.start);
        return TYPE_ERROR;
    }
    if (count != fn->param_count) {
        error(c, node->offset, "'%.*s' takes %d argument%s, found %d",
              callee.length, callee.start, fn->param_count,
              fn->param_count == 1 ? "" : "s", count);
        return fn->returns;
    }
    for (int i = 0; i < count; i++) {
        Type want = fn->params[i].type;
        if (args[i]->ty != TYPE_ERROR && args[i]->ty != want) {
            Name param = fn->params[i].name;
            error(c, start_of(args[i]), "argument '%.*s' of '%.*s' must be %s, found %s",
                  param.length, param.start, callee.length, callee.start,
                  type_name(want), type_name(args[i]->ty));
        }
    }
    return fn->returns;
}

static Type expr_type(Checker *c, Node *node) {
    switch (node->type) {
    case NODE_NUMBER:
        return TYPE_INT;
    case NODE_BOOL:
        return TYPE_BOOL;
    case NODE_NAME: {
        Name name = node->as.name;
        Local *local = find_local(c, name);
        if (local != NULL) return local->type;
        if (find_function(c, name) != NULL) {
            error(c, node->offset, "'%.*s' is a function, call it with %.*s(...)",
                  name.length, name.start, name.length, name.start);
        } else {
            error(c, node->offset, "undefined name '%.*s'", name.length, name.start);
        }
        return TYPE_ERROR;
    }
    case NODE_UNARY: {
        Type want = node->as.unary.op == TOK_BANG ? TYPE_BOOL : TYPE_INT;
        Type found = check_value(c, node->as.unary.operand);
        if (found != TYPE_ERROR && found != want) {
            error(c, node->offset, "'%s' needs %s, found %s",
                  op_symbol(node->as.unary.op), type_name(want), type_name(found));
        }
        return want;
    }
    case NODE_BINARY:
        return check_binary(c, node);
    case NODE_CALL:
        return check_call(c, node);
    }
    return TYPE_ERROR;
}

static Type check_expr(Checker *c, Node *node) {
    node->ty = expr_type(c, node);
    return node->ty;
}

static bool check_block(Checker *c, Stmt *body);

static bool check_stmt(Checker *c, Stmt *stmt) {
    switch (stmt->type) {
    case STMT_LET: {
        Name name = stmt->as.let.name;
        Type type = check_value(c, stmt->as.let.value);
        if (stmt->as.let.annotated) {
            if (type != TYPE_ERROR && type != stmt->as.let.annotation) {
                error(c, start_of(stmt->as.let.value), "'%.*s' is declared %s, found %s",
                      name.length, name.start,
                      type_name(stmt->as.let.annotation), type_name(type));
            }
            type = stmt->as.let.annotation;
        }
        declare(c, name, type, stmt->as.let.mutable);
        return false;
    }
    case STMT_ASSIGN: {
        Name name = stmt->as.assign.name;
        Type type = check_value(c, stmt->as.assign.value);
        Local *local = find_local(c, name);
        if (local == NULL) {
            error(c, stmt->offset, "undefined name '%.*s'", name.length, name.start);
        } else if (!local->mutable) {
            error(c, stmt->offset, "cannot assign to '%.*s', it was not declared with var",
                  name.length, name.start);
        } else if (local->type != TYPE_ERROR && type != TYPE_ERROR && type != local->type) {
            error(c, start_of(stmt->as.assign.value), "'%.*s' is %s, found %s",
                  name.length, name.start, type_name(local->type), type_name(type));
        }
        return false;
    }
    case STMT_PRINT:
        check_value(c, stmt->as.expr);
        return false;
    case STMT_IF: {
        Node *cond = stmt->as.if_.cond;
        expect_type(c, cond, check_value(c, cond), TYPE_BOOL, "condition");
        bool then_returns = check_block(c, stmt->as.if_.then_body);
        bool else_returns = check_block(c, stmt->as.if_.else_body);
        return then_returns && else_returns && stmt->as.if_.else_body != NULL;
    }
    case STMT_WHILE: {
        Node *cond = stmt->as.while_.cond;
        expect_type(c, cond, check_value(c, cond), TYPE_BOOL, "condition");
        check_block(c, stmt->as.while_.body);
        return cond->type == NODE_BOOL && cond->as.boolean;
    }
    case STMT_RETURN: {
        Node *value = stmt->as.expr;
        const Function *fn = c->fn;
        if (fn == NULL) {
            error(c, stmt->offset, "return outside of a function");
            if (value != NULL) check_value(c, value);
            return false;
        }
        if (value == NULL) {
            if (fn->returns != TYPE_VOID) {
                error(c, stmt->offset, "'%.*s' must return %s",
                      fn->name.length, fn->name.start, type_name(fn->returns));
            }
            return true;
        }
        Type type = check_value(c, value);
        if (fn->returns == TYPE_VOID) {
            error(c, start_of(value), "'%.*s' has no return type, so it cannot return a value",
                  fn->name.length, fn->name.start);
        } else if (type != TYPE_ERROR && type != fn->returns) {
            error(c, start_of(value), "'%.*s' returns %s, found %s",
                  fn->name.length, fn->name.start, type_name(fn->returns), type_name(type));
        }
        return true;
    }
    case STMT_EXPR:
        check_expr(c, stmt->as.expr);
        if (stmt->as.expr->type != NODE_CALL) {
            error(c, start_of(stmt->as.expr), "only a function call can be used as a statement");
        }
        return false;
    }
    return false;
}

static bool check_block(Checker *c, Stmt *body) {
    int mark = c->count;
    bool returns = false;
    bool warned = false;

    for (Stmt *stmt = body; stmt != NULL; stmt = stmt->next) {
        if (returns && !warned) {
            diag_warning(c->src, stmt->offset, "unreachable code");
            warned = true;
        }
        if (check_stmt(c, stmt)) returns = true;
    }

    c->count = mark;
    return returns;
}

static void check_function(Checker *c, const Function *fn) {
    for (const Function *other = c->prog->functions; other != fn; other = other->next) {
        if (name_eq(other->name, fn->name)) {
            error(c, fn->offset, "function '%.*s' is already defined",
                  fn->name.length, fn->name.start);
            break;
        }
    }

    c->fn = fn;
    c->count = 0;
    for (int i = 0; i < fn->param_count; i++) {
        declare(c, fn->params[i].name, fn->params[i].type, false);
    }

    bool returns = check_block(c, fn->body);
    if (!returns && fn->returns != TYPE_VOID) {
        error(c, fn->offset, "'%.*s' can reach its end without returning %s",
              fn->name.length, fn->name.start, type_name(fn->returns));
    }

    c->fn = NULL;
    c->count = 0;
}

static Checker new_checker(const char *src, const Program *prog) {
    Checker c = {src, prog, NULL, NULL, 0, 0, 0};
    return c;
}

bool check_program(const char *src, Program *prog) {
    Checker top = new_checker(src, prog);
    Checker inner = new_checker(src, prog);
    Function *fn = prog->functions;
    Stmt *stmt = prog->statements;
    bool returned = false;
    bool warned = false;

    while (fn != NULL || stmt != NULL) {
        if (fn != NULL && (stmt == NULL || fn->offset < stmt->offset)) {
            check_function(&inner, fn);
            fn = fn->next;
            continue;
        }
        if (returned && !warned) {
            diag_warning(src, stmt->offset, "unreachable code");
            warned = true;
        }
        if (check_stmt(&top, stmt)) returned = true;
        stmt = stmt->next;
    }

    int errors = top.errors + inner.errors;
    free(top.locals);
    free(inner.locals);
    return errors == 0;
}

bool check_expression(const char *src, Node *expr) {
    Checker c = new_checker(src, NULL);
    check_value(&c, expr);
    free(c.locals);
    return c.errors == 0;
}
