#include <assert.h>
#include <stdio.h>
#include "../src/parser.h"

static int count(const Stmt *s) {
    int n = 0;
    for (; s != NULL; s = s->next) n++;
    return n;
}

int main(void) {
    Parser p;
    Program prog;

    if (freopen("/dev/null", "w", stderr) == NULL) return 1;

    parser_init(&p, "let x = 1; var y: Int = 2; y = x + y; print(y);");
    prog = parser_parse_program(&p);
    assert(!p.had_error);
    assert(count(prog.statements) == 4);
    Stmt *s = prog.statements;
    assert(s->type == STMT_LET && !s->as.let.mutable && !s->as.let.annotated);
    assert(name_is(s->as.let.name, "x"));
    s = s->next;
    assert(s->type == STMT_LET && s->as.let.mutable);
    assert(s->as.let.annotated && s->as.let.annotation == TYPE_INT);
    s = s->next;
    assert(s->type == STMT_ASSIGN && name_is(s->as.assign.name, "y"));
    assert(s->as.assign.value->type == NODE_BINARY);
    s = s->next;
    assert(s->type == STMT_PRINT && s->as.expr->type == NODE_NAME);
    program_free(&prog);

    parser_init(&p, "fn add(a: Int, b: Int) -> Int { return a + b; } fn nothing() { }");
    prog = parser_parse_program(&p);
    assert(!p.had_error);
    Function *fn = prog.functions;
    assert(fn != NULL && name_is(fn->name, "add"));
    assert(fn->param_count == 2 && fn->returns == TYPE_INT);
    assert(name_is(fn->params[1].name, "b") && fn->params[1].type == TYPE_INT);
    assert(fn->body->type == STMT_RETURN);
    fn = fn->next;
    assert(fn != NULL && fn->param_count == 0 && fn->returns == TYPE_VOID && fn->body == NULL);
    assert(fn->next == NULL);
    program_free(&prog);

    parser_init(&p, "if a { } else if b { } else { print(1); }");
    prog = parser_parse_program(&p);
    assert(!p.had_error);
    s = prog.statements;
    assert(s->type == STMT_IF && s->as.if_.then_body == NULL);
    assert(s->as.if_.else_body->type == STMT_IF);
    assert(s->as.if_.else_body->as.if_.else_body->type == STMT_PRINT);
    program_free(&prog);

    parser_init(&p, "f(1, g(2), 3);");
    prog = parser_parse_program(&p);
    assert(!p.had_error);
    Node *call = prog.statements->as.expr;
    assert(call->type == NODE_CALL && call->as.call.arg_count == 3);
    assert(call->as.call.args[1]->type == NODE_CALL);
    program_free(&prog);

    parser_init(&p, "a || b && c == d < e + f * -g");
    Node *root = parser_parse(&p);
    Node *t = root;
    assert(t->type == NODE_BINARY && t->as.binary.op == TOK_OR);
    t = t->as.binary.right;
    assert(t->as.binary.op == TOK_AND);
    t = t->as.binary.right;
    assert(t->as.binary.op == TOK_EQUAL_EQUAL);
    t = t->as.binary.right;
    assert(t->as.binary.op == TOK_LESS);
    t = t->as.binary.right;
    assert(t->as.binary.op == TOK_PLUS);
    t = t->as.binary.right;
    assert(t->as.binary.op == TOK_STAR);
    assert(t->as.binary.right->type == NODE_UNARY);
    ast_free(root);

    parser_init(&p, "let = 1; print(2); let b = ; print(3);");
    prog = parser_parse_program(&p);
    assert(p.had_error);
    assert(count(prog.statements) == 2);
    program_free(&prog);

    parser_init(&p, "fn f() { let = ; print(1); } print(2);");
    prog = parser_parse_program(&p);
    assert(p.had_error);
    assert(prog.functions != NULL && count(prog.functions->body) == 1);
    assert(count(prog.statements) == 1);
    program_free(&prog);

    parser_init(&p, "x + 1 = 2;");
    prog = parser_parse_program(&p);
    assert(p.had_error && prog.statements == NULL);
    program_free(&prog);

    printf("test_parser_3 passed\n");
    return 0;
}
