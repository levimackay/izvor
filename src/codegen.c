#include <stdbool.h>
#include <stdlib.h>
#include "codegen.h"
#include "diag.h"

typedef struct {
    const Node *node;
    int id;
} Temp;

typedef struct {
    FILE *out;
    const char *src;
    int indent;
    int next_temp;
    Temp *temps;
    int temp_count;
    int temp_capacity;
} Gen;

static const char prelude[] =
    "#include <limits.h>\n"
    "#include <stdbool.h>\n"
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "\n"
    "static void rt_fail(const char *message, const char *at) {\n"
    "    fflush(stdout);\n"
    "    fprintf(stderr, \"error: %s\\n --> %s\\n\", message, at);\n"
    "    exit(1);\n"
    "}\n"
    "\n"
    "static long rt_add(long a, long b, const char *at) {\n"
    "    long r;\n"
    "    if (__builtin_add_overflow(a, b, &r)) rt_fail(\"integer overflow\", at);\n"
    "    return r;\n"
    "}\n"
    "\n"
    "static long rt_sub(long a, long b, const char *at) {\n"
    "    long r;\n"
    "    if (__builtin_sub_overflow(a, b, &r)) rt_fail(\"integer overflow\", at);\n"
    "    return r;\n"
    "}\n"
    "\n"
    "static long rt_mul(long a, long b, const char *at) {\n"
    "    long r;\n"
    "    if (__builtin_mul_overflow(a, b, &r)) rt_fail(\"integer overflow\", at);\n"
    "    return r;\n"
    "}\n"
    "\n"
    "static long rt_div(long a, long b, const char *at) {\n"
    "    if (b == 0) rt_fail(\"division by zero\", at);\n"
    "    if (a == LONG_MIN && b == -1) rt_fail(\"integer overflow\", at);\n"
    "    return a / b;\n"
    "}\n"
    "\n"
    "static long rt_mod(long a, long b, const char *at) {\n"
    "    if (b == 0) rt_fail(\"division by zero\", at);\n"
    "    if (a == LONG_MIN && b == -1) rt_fail(\"integer overflow\", at);\n"
    "    return a % b;\n"
    "}\n"
    "\n"
    "static long rt_neg(long a, const char *at) {\n"
    "    if (a == LONG_MIN) rt_fail(\"integer overflow\", at);\n"
    "    return -a;\n"
    "}\n"
    "\n"
    "static void rt_print_int(long value) {\n"
    "    printf(\"%ld\\n\", value);\n"
    "}\n"
    "\n"
    "static void rt_print_bool(bool value) {\n"
    "    puts(value ? \"true\" : \"false\");\n"
    "}\n";

static void emit_expr(Gen *g, const Node *node, bool nested);
static void emit_block(Gen *g, const Stmt *body);

static void indent(Gen *g) {
    for (int i = 0; i < g->indent; i++) {
        fputs("    ", g->out);
    }
}

static void emit_name(Gen *g, Name name) {
    fprintf(g->out, "iz_%.*s", name.length, name.start);
}

static const char *c_type(Type type) {
    switch (type) {
    case TYPE_INT: return "long";
    case TYPE_BOOL: return "bool";
    case TYPE_VOID: return "void";
    case TYPE_ERROR: break;
    }
    return "long";
}

static void emit_at(Gen *g, long offset) {
    char at[4096];
    diag_location(g->src, offset, at, sizeof at);

    fputs(", \"", g->out);
    for (const char *s = at; *s != '\0'; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\') {
            fprintf(g->out, "\\%c", c);
        } else if (c < 0x20 || c >= 0x7f) {
            fprintf(g->out, "\\%03o", c);
        } else {
            fputc(c, g->out);
        }
    }
    fputc('"', g->out);
}

static const char *helper_for(TokenType op) {
    switch (op) {
    case TOK_PLUS: return "rt_add";
    case TOK_MINUS: return "rt_sub";
    case TOK_STAR: return "rt_mul";
    case TOK_SLASH: return "rt_div";
    case TOK_PERCENT: return "rt_mod";
    default: return NULL;
    }
}

static int operand_count(const Node *node) {
    switch (node->type) {
    case NODE_UNARY: return 1;
    case NODE_BINARY: return 2;
    case NODE_CALL: return node->as.call.arg_count;
    case NODE_NUMBER:
    case NODE_BOOL:
    case NODE_NAME:
        break;
    }
    return 0;
}

static const Node *operand(const Node *node, int i) {
    switch (node->type) {
    case NODE_UNARY: return node->as.unary.operand;
    case NODE_BINARY: return i == 0 ? node->as.binary.left : node->as.binary.right;
    case NODE_CALL: return node->as.call.args[i];
    case NODE_NUMBER:
    case NODE_BOOL:
    case NODE_NAME:
        break;
    }
    return NULL;
}

static bool short_circuits(const Node *node) {
    return node->type == NODE_BINARY &&
           (node->as.binary.op == TOK_AND || node->as.binary.op == TOK_OR);
}

static bool has_effects(const Node *node) {
    if (node->type == NODE_CALL) return true;
    if (node->type == NODE_UNARY && node->as.unary.op == TOK_MINUS &&
        node->as.unary.operand->type != NODE_NUMBER) return true;
    if (node->type == NODE_BINARY && helper_for(node->as.binary.op) != NULL) return true;

    for (int i = 0; i < operand_count(node); i++) {
        if (has_effects(operand(node, i))) return true;
    }
    return false;
}

static bool needs_temps(const Node *node) {
    if (short_circuits(node)) {
        return needs_temps(node->as.binary.left) || needs_temps(node->as.binary.right);
    }

    int effectful = 0;
    for (int i = 0; i < operand_count(node); i++) {
        const Node *kid = operand(node, i);
        if (needs_temps(kid)) return true;
        if (has_effects(kid)) effectful++;
    }
    return effectful > 1;
}

static int find_temp(const Gen *g, const Node *node) {
    for (int i = 0; i < g->temp_count; i++) {
        if (g->temps[i].node == node) return g->temps[i].id;
    }
    return 0;
}

static int hoist(Gen *g, const Node *node) {
    int id = ++g->next_temp;
    indent(g);
    fprintf(g->out, "%s t%d = ", c_type(node->ty), id);
    emit_expr(g, node, false);
    fputs(";\n", g->out);

    if (g->temp_count == g->temp_capacity) {
        g->temp_capacity = g->temp_capacity == 0 ? 8 : g->temp_capacity * 2;
        g->temps = xrealloc(g->temps, sizeof(Temp) * (size_t)g->temp_capacity);
    }
    g->temps[g->temp_count].node = node;
    g->temps[g->temp_count].id = id;
    return g->temp_count++;
}

static void prepare(Gen *g, const Node *node) {
    if (short_circuits(node)) {
        const Node *left = node->as.binary.left;
        const Node *right = node->as.binary.right;
        prepare(g, left);
        if (!needs_temps(right)) return;

        int slot = hoist(g, left);
        int id = g->temps[slot].id;
        indent(g);
        fprintf(g->out, "if (%st%d) {\n", node->as.binary.op == TOK_AND ? "" : "!", id);
        g->indent++;
        prepare(g, right);
        indent(g);
        fprintf(g->out, "t%d = ", id);
        emit_expr(g, right, false);
        fputs(";\n", g->out);
        g->indent--;
        indent(g);
        fputs("}\n", g->out);
        g->temps[slot].node = node;
        return;
    }

    int effectful = 0;
    for (int i = 0; i < operand_count(node); i++) {
        if (has_effects(operand(node, i))) effectful++;
    }

    int seen = 0;
    for (int i = 0; i < operand_count(node); i++) {
        const Node *kid = operand(node, i);
        prepare(g, kid);
        if (!has_effects(kid)) continue;
        seen++;
        if (seen < effectful && find_temp(g, kid) == 0) hoist(g, kid);
    }
}

static void emit_binary(Gen *g, const Node *node, bool nested) {
    const char *helper = helper_for(node->as.binary.op);
    if (helper != NULL) {
        fprintf(g->out, "%s(", helper);
        emit_expr(g, node->as.binary.left, false);
        fputs(", ", g->out);
        emit_expr(g, node->as.binary.right, false);
        emit_at(g, node->offset);
        fputc(')', g->out);
        return;
    }

    if (nested) fputc('(', g->out);
    emit_expr(g, node->as.binary.left, true);
    fprintf(g->out, " %s ", op_symbol(node->as.binary.op));
    emit_expr(g, node->as.binary.right, true);
    if (nested) fputc(')', g->out);
}

static void emit_expr(Gen *g, const Node *node, bool nested) {
    int temp = find_temp(g, node);
    if (temp != 0) {
        fprintf(g->out, "t%d", temp);
        return;
    }

    switch (node->type) {
    case NODE_NUMBER:
        fprintf(g->out, "%ld", node->as.number);
        break;
    case NODE_BOOL:
        fputs(node->as.boolean ? "true" : "false", g->out);
        break;
    case NODE_NAME:
        emit_name(g, node->as.name);
        break;
    case NODE_UNARY:
        if (node->as.unary.op == TOK_BANG) {
            fputc('!', g->out);
            emit_expr(g, node->as.unary.operand, true);
        } else if (node->as.unary.operand->type == NODE_NUMBER) {
            fprintf(g->out, "-%ld", node->as.unary.operand->as.number);
        } else {
            fputs("rt_neg(", g->out);
            emit_expr(g, node->as.unary.operand, false);
            emit_at(g, node->offset);
            fputc(')', g->out);
        }
        break;
    case NODE_BINARY:
        emit_binary(g, node, nested);
        break;
    case NODE_CALL:
        emit_name(g, node->as.call.callee);
        fputc('(', g->out);
        for (int i = 0; i < node->as.call.arg_count; i++) {
            if (i > 0) fputs(", ", g->out);
            emit_expr(g, node->as.call.args[i], false);
        }
        fputc(')', g->out);
        break;
    }
}

static void start_stmt(Gen *g, const Node *expr) {
    g->temp_count = 0;
    if (expr != NULL) prepare(g, expr);
    indent(g);
}

static void emit_if(Gen *g, const Stmt *stmt) {
    fputs("if (", g->out);
    emit_expr(g, stmt->as.if_.cond, false);
    fputs(") {\n", g->out);
    emit_block(g, stmt->as.if_.then_body);

    const Stmt *other = stmt->as.if_.else_body;
    if (other == NULL) {
        indent(g);
        fputs("}\n", g->out);
    } else if (other->type == STMT_IF && other->next == NULL && !needs_temps(other->as.if_.cond)) {
        indent(g);
        fputs("} else ", g->out);
        g->temp_count = 0;
        emit_if(g, other);
    } else {
        indent(g);
        fputs("} else {\n", g->out);
        emit_block(g, other);
        indent(g);
        fputs("}\n", g->out);
    }
}

static void emit_while(Gen *g, const Stmt *stmt) {
    const Node *cond = stmt->as.while_.cond;
    if (!needs_temps(cond)) {
        start_stmt(g, NULL);
        fputs("while (", g->out);
        emit_expr(g, cond, false);
        fputs(") {\n", g->out);
    } else {
        start_stmt(g, NULL);
        fputs("while (true) {\n", g->out);
        g->indent++;
        start_stmt(g, cond);
        fputs("if (!", g->out);
        emit_expr(g, cond, true);
        fputs(") break;\n", g->out);
        g->indent--;
    }
    emit_block(g, stmt->as.while_.body);
    indent(g);
    fputs("}\n", g->out);
}

static void emit_stmt(Gen *g, const Stmt *stmt) {
    switch (stmt->type) {
    case STMT_LET:
        start_stmt(g, stmt->as.let.value);
        fprintf(g->out, "%s%s ", stmt->as.let.mutable ? "" : "const ",
                c_type(stmt->as.let.annotated ? stmt->as.let.annotation : stmt->as.let.value->ty));
        emit_name(g, stmt->as.let.name);
        fputs(" = ", g->out);
        emit_expr(g, stmt->as.let.value, false);
        fputs(";\n", g->out);
        break;
    case STMT_ASSIGN:
        start_stmt(g, stmt->as.assign.value);
        emit_name(g, stmt->as.assign.name);
        fputs(" = ", g->out);
        emit_expr(g, stmt->as.assign.value, false);
        fputs(";\n", g->out);
        break;
    case STMT_PRINT:
        start_stmt(g, stmt->as.expr);
        fputs(stmt->as.expr->ty == TYPE_BOOL ? "rt_print_bool(" : "rt_print_int(", g->out);
        emit_expr(g, stmt->as.expr, false);
        fputs(");\n", g->out);
        break;
    case STMT_IF:
        start_stmt(g, stmt->as.if_.cond);
        emit_if(g, stmt);
        break;
    case STMT_WHILE:
        emit_while(g, stmt);
        break;
    case STMT_RETURN:
        start_stmt(g, stmt->as.expr);
        if (stmt->as.expr == NULL) {
            fputs("return;\n", g->out);
        } else {
            fputs("return ", g->out);
            emit_expr(g, stmt->as.expr, false);
            fputs(";\n", g->out);
        }
        break;
    case STMT_EXPR:
        start_stmt(g, stmt->as.expr);
        emit_expr(g, stmt->as.expr, false);
        fputs(";\n", g->out);
        break;
    }
}

static void emit_block(Gen *g, const Stmt *body) {
    g->indent++;
    for (const Stmt *stmt = body; stmt != NULL; stmt = stmt->next) {
        emit_stmt(g, stmt);
    }
    g->indent--;
}

static void emit_signature(Gen *g, const Function *fn) {
    fprintf(g->out, "static %s ", c_type(fn->returns));
    emit_name(g, fn->name);
    fputc('(', g->out);
    if (fn->param_count == 0) fputs("void", g->out);
    for (int i = 0; i < fn->param_count; i++) {
        if (i > 0) fputs(", ", g->out);
        fprintf(g->out, "%s ", c_type(fn->params[i].type));
        emit_name(g, fn->params[i].name);
    }
    fputc(')', g->out);
}

void codegen_emit(FILE *out, const char *src, const Program *prog) {
    Gen g = {out, src, 0, 0, NULL, 0, 0};

    fputs(prelude, out);

    if (prog->functions != NULL) fputc('\n', out);
    for (const Function *fn = prog->functions; fn != NULL; fn = fn->next) {
        emit_signature(&g, fn);
        fputs(";\n", out);
    }

    for (const Function *fn = prog->functions; fn != NULL; fn = fn->next) {
        fputc('\n', out);
        emit_signature(&g, fn);
        fputs(" {\n", out);
        g.next_temp = 0;
        emit_block(&g, fn->body);
        fputs("}\n", out);
    }

    fputs("\nint main(void) {\n", out);
    g.next_temp = 0;
    emit_block(&g, prog->statements);
    fputs("    return 0;\n}\n", out);

    free(g.temps);
}
