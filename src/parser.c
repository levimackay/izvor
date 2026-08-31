
/* parser_init  — set up the lexer, then advance once so
                  current is loaded before anyone reads it */

/* parser_advance — pull the next token into current */

/* parser_check — is current this type? asks only, consumes nothing */

/* parser_match — if current is this type, consume it and say true; otherwise change nothing and say false */


/* Parser implementation: token helpers and a recursive-descent
   expression parser (expression / term / factor). */

#include <stdio.h>
#include "parser.h"

void parser_init(Parser *p, const char *src) {
    lexer_init(&p->lexer, src);
    parser_advance(p);
}

void parser_advance(Parser *p) {
    p->current = lexer_next(&p->lexer);
}

bool parser_check(const Parser *p, TokenType type) {
    return p->current.type == type;
}

bool parser_match(Parser *p, TokenType type) {
    if (!parser_check(p, type)) return false;
    parser_advance(p);
    return true;
}

/* error_at — report a parse error with its character offset, naming
   what was expected and what was actually there. */
static void error_at(const Parser *p, const char *msg) {
    long offset = p->current.start - p->lexer.src;
    fprintf(stderr, "parse error at offset %ld: %s (got %s)\n",
            offset, msg, token_type_name(p->current.type));
}

static Node *parse_expression(Parser *p);

/* factor -> NUMBER | "(" expression ")" | "-" factor */
static Node *parse_factor(Parser *p) {
    if (parser_check(p, TOK_NUMBER)) {
        long value = p->current.value;
        parser_advance(p);
        return ast_number(value);
    }
    if (parser_match(p, TOK_LPAREN)) {
        Node *inner = parse_expression(p);
        if (inner == NULL) return NULL;
        if (!parser_match(p, TOK_RPAREN)) {
            error_at(p, "expected ')'");
            ast_free(inner);
            return NULL;
        }
        return inner;
    }
    if (parser_match(p, TOK_MINUS)) {
        Node *operand = parse_factor(p);
        if (operand == NULL) return NULL;
        return ast_unary(TOK_MINUS, operand);
    }
    error_at(p, "expected expression");
    return NULL;
}

/* term -> factor (("*" | "/") factor)* */
static Node *parse_term(Parser *p) {
    Node *left = parse_factor(p);
    if (left == NULL) return NULL;
    while (parser_check(p, TOK_STAR) || parser_check(p, TOK_SLASH)) {
        TokenType op = p->current.type;
        parser_advance(p);
        Node *right = parse_factor(p);
        if (right == NULL) {
            ast_free(left);
            return NULL;
        }
        left = ast_binary(op, left, right);
    }
    return left;
}

/* expression -> term (("+" | "-") term)* */
static Node *parse_expression(Parser *p) {
    Node *left = parse_term(p);
    if (left == NULL) return NULL;
    while (parser_check(p, TOK_PLUS) || parser_check(p, TOK_MINUS)) {
        TokenType op = p->current.type;
        parser_advance(p);
        Node *right = parse_term(p);
        if (right == NULL) {
            ast_free(left);
            return NULL;
        }
        left = ast_binary(op, left, right);
    }
    return left;
}

Node *parser_parse(Parser *p) {
    Node *expr = parse_expression(p);
    if (expr == NULL) return NULL;
    if (!parser_check(p, TOK_EOF)) {
        error_at(p, "expected end of input");
        ast_free(expr);
        return NULL;
    }
    return expr;
}