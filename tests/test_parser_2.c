#include <assert.h>
#include <stdio.h>
#include "../src/parser.h"

static Node *parse(Parser *p, const char *src) {
    parser_init(p, src);
    return parser_parse(p);
}

static bool is_number(const Node *n, long value) {
    return n != NULL && n->type == NODE_NUMBER && n->as.number == value;
}

static bool is_binary(const Node *n, TokenType op) {
    return n != NULL && n->type == NODE_BINARY && n->as.binary.op == op;
}

int main(void) {
    Parser p;
    Node *t;

    t = parse(&p, "42");
    assert(is_number(t, 42));
    ast_free(t);

    t = parse(&p, "2+3*4");
    assert(is_binary(t, TOK_PLUS));
    assert(is_number(t->as.binary.left, 2));
    assert(is_binary(t->as.binary.right, TOK_STAR));
    assert(is_number(t->as.binary.right->as.binary.left, 3));
    assert(is_number(t->as.binary.right->as.binary.right, 4));
    ast_free(t);

    t = parse(&p, "10-3-2");
    assert(is_binary(t, TOK_MINUS));
    assert(is_binary(t->as.binary.left, TOK_MINUS));
    assert(is_number(t->as.binary.left->as.binary.left, 10));
    assert(is_number(t->as.binary.left->as.binary.right, 3));
    assert(is_number(t->as.binary.right, 2));
    ast_free(t);

    t = parse(&p, "(2+3)*4");
    assert(is_binary(t, TOK_STAR));
    assert(is_binary(t->as.binary.left, TOK_PLUS));
    assert(is_number(t->as.binary.right, 4));
    ast_free(t);

    t = parse(&p, "-5*2");
    assert(is_binary(t, TOK_STAR));
    assert(t->as.binary.left->type == NODE_UNARY);
    assert(t->as.binary.left->as.unary.op == TOK_MINUS);
    assert(is_number(t->as.binary.left->as.unary.operand, 5));
    ast_free(t);

    t = parse(&p, "--7");
    assert(t != NULL && t->type == NODE_UNARY);
    assert(t->as.unary.operand->type == NODE_UNARY);
    assert(is_number(t->as.unary.operand->as.unary.operand, 7));
    ast_free(t);

    assert(parse(&p, "1+") == NULL);
    assert(parse(&p, "(1+2") == NULL);
    assert(parse(&p, "1 2") == NULL);
    assert(parse(&p, "") == NULL);
    assert(parse(&p, "*3") == NULL);
    assert(parse(&p, "()") == NULL);

    printf("test_parser_2 passed\n");
    return 0;
}
