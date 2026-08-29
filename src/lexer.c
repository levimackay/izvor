/* Lexer implementation: scans characters into tokens.
   Handles integer literals, + - * / and parentheses; anything
   else becomes TOK_ERROR rather than aborting the scan. */

#include <ctype.h>
#include "lexer.h"

const char *token_type_name(TokenType type) {
    switch (type) {
    case TOK_NUMBER:
        return "NUMBER";
    case TOK_PLUS:
        return "PLUS";
    case TOK_MINUS:
        return "MINUS";
    case TOK_STAR:
        return "STAR";
    case TOK_SLASH:
        return "SLASH";
    case TOK_LPAREN:
        return "LPAREN";
    case TOK_RPAREN:
        return "RPAREN";
    case TOK_EOF:
        return "EOF";
    case TOK_ERROR:
        return "ERROR";
    }
    return "???";
}

void lexer_init(Lexer *lx, const char *src) {
    lx->src = src;
    lx->pos = 0;
}

static char peek(const Lexer *lx) {
    return lx->src[lx->pos];
}

static char advance(Lexer *lx) {
    return lx->src[lx->pos++];
}

static Token make_token(const Lexer *lx, TokenType type, int start_pos) {
    Token t;
    t.type = type;
    t.start = lx->src + start_pos;
    t.length = lx->pos - start_pos;
    t.value = 0;
    return t;
}

static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

static void skip_whitespace(Lexer *lx) {
    while (isspace((unsigned char)peek(lx))) advance(lx);
}

Token lexer_next(Lexer *lx) {
    skip_whitespace(lx);

    int start = lx->pos;
    char c = peek(lx);

    if (c == '\0') return make_token(lx, TOK_EOF, start);

    if (is_digit(c)) {
        long value = 0;
        while (is_digit(peek(lx))) value = value * 10 + (advance(lx) - '0');
        Token t = make_token(lx, TOK_NUMBER, start);
        t.value = value;
        return t;
    }

    advance(lx);
    switch (c) {
        case '+': return make_token(lx, TOK_PLUS,   start);
        case '-': return make_token(lx, TOK_MINUS,  start);
        case '*': return make_token(lx, TOK_STAR,   start);
        case '/': return make_token(lx, TOK_SLASH,  start);
        case '(': return make_token(lx, TOK_LPAREN, start);
        case ')': return make_token(lx, TOK_RPAREN, start);
    }
    return make_token(lx, TOK_ERROR, start);
}
