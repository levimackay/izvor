#include <ctype.h>
#include <limits.h>
#include <string.h>
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
    case TOK_PERCENT:
        return "PERCENT";
    case TOK_LPAREN:
        return "LPAREN";
    case TOK_RPAREN:
        return "RPAREN";
    case TOK_LBRACE:
        return "LBRACE";
    case TOK_RBRACE:
        return "RBRACE";
    case TOK_COMMA:
        return "COMMA";
    case TOK_COLON:
        return "COLON";
    case TOK_SEMICOLON:
        return "SEMICOLON";
    case TOK_ARROW:
        return "ARROW";
    case TOK_EQUAL:
        return "EQUAL";
    case TOK_EQUAL_EQUAL:
        return "EQUAL_EQUAL";
    case TOK_BANG:
        return "BANG";
    case TOK_BANG_EQUAL:
        return "BANG_EQUAL";
    case TOK_LESS:
        return "LESS";
    case TOK_LESS_EQUAL:
        return "LESS_EQUAL";
    case TOK_GREATER:
        return "GREATER";
    case TOK_GREATER_EQUAL:
        return "GREATER_EQUAL";
    case TOK_AND:
        return "AND";
    case TOK_OR:
        return "OR";
    case TOK_IDENT:
        return "IDENT";
    case TOK_PRINT:
        return "PRINT";
    case TOK_LET:
        return "LET";
    case TOK_VAR:
        return "VAR";
    case TOK_FN:
        return "FN";
    case TOK_RETURN:
        return "RETURN";
    case TOK_IF:
        return "IF";
    case TOK_ELSE:
        return "ELSE";
    case TOK_WHILE:
        return "WHILE";
    case TOK_TRUE:
        return "TRUE";
    case TOK_FALSE:
        return "FALSE";
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

static char peek_next(const Lexer *lx) {
    if (lx->src[lx->pos] == '\0') return '\0';
    return lx->src[lx->pos + 1];
}

static char advance(Lexer *lx) {
    return lx->src[lx->pos++];
}

static int match(Lexer *lx, char expected) {
    if (peek(lx) != expected) return 0;
    lx->pos++;
    return 1;
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

static int is_ident_start(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static void skip_whitespace(Lexer *lx) {
    for (;;) {
        char c = peek(lx);
        if (isspace((unsigned char)c)) {
            advance(lx);
        } else if (c == '/' && peek_next(lx) == '/') {
            while (peek(lx) != '\n' && peek(lx) != '\0') advance(lx);
        } else {
            return;
        }
    }
}

static const struct {
    const char *text;
    TokenType type;
} keywords[] = {
    {"print", TOK_PRINT},
    {"let", TOK_LET},
    {"var", TOK_VAR},
    {"fn", TOK_FN},
    {"return", TOK_RETURN},
    {"if", TOK_IF},
    {"else", TOK_ELSE},
    {"while", TOK_WHILE},
    {"true", TOK_TRUE},
    {"false", TOK_FALSE},
};

static TokenType classify(const char *start, int length) {
    for (size_t i = 0; i < sizeof keywords / sizeof keywords[0]; i++) {
        if ((int)strlen(keywords[i].text) == length &&
            memcmp(keywords[i].text, start, (size_t)length) == 0) {
            return keywords[i].type;
        }
    }
    return TOK_IDENT;
}

static Token number(Lexer *lx, int start) {
    long value = 0;
    int too_big = 0;
    while (is_digit(peek(lx))) {
        int digit = advance(lx) - '0';
        if (value > (LONG_MAX - digit) / 10) too_big = 1;
        if (!too_big) value = value * 10 + digit;
    }
    if (too_big) return make_token(lx, TOK_ERROR, start);
    Token t = make_token(lx, TOK_NUMBER, start);
    t.value = value;
    return t;
}

Token lexer_next(Lexer *lx) {
    skip_whitespace(lx);

    int start = lx->pos;
    char c = peek(lx);

    if (c == '\0') return make_token(lx, TOK_EOF, start);

    if (is_digit(c)) return number(lx, start);

    if (is_ident_start(c)) {
        while (is_ident_start(peek(lx)) || is_digit(peek(lx))) advance(lx);
        return make_token(lx, classify(lx->src + start, lx->pos - start), start);
    }

    advance(lx);
    switch (c) {
    case '+': return make_token(lx, TOK_PLUS,      start);
    case '*': return make_token(lx, TOK_STAR,      start);
    case '/': return make_token(lx, TOK_SLASH,     start);
    case '%': return make_token(lx, TOK_PERCENT,   start);
    case '(': return make_token(lx, TOK_LPAREN,    start);
    case ')': return make_token(lx, TOK_RPAREN,    start);
    case '{': return make_token(lx, TOK_LBRACE,    start);
    case '}': return make_token(lx, TOK_RBRACE,    start);
    case ',': return make_token(lx, TOK_COMMA,     start);
    case ':': return make_token(lx, TOK_COLON,     start);
    case ';': return make_token(lx, TOK_SEMICOLON, start);
    case '-':
        return make_token(lx, match(lx, '>') ? TOK_ARROW : TOK_MINUS, start);
    case '=':
        return make_token(lx, match(lx, '=') ? TOK_EQUAL_EQUAL : TOK_EQUAL, start);
    case '!':
        return make_token(lx, match(lx, '=') ? TOK_BANG_EQUAL : TOK_BANG, start);
    case '<':
        return make_token(lx, match(lx, '=') ? TOK_LESS_EQUAL : TOK_LESS, start);
    case '>':
        return make_token(lx, match(lx, '=') ? TOK_GREATER_EQUAL : TOK_GREATER, start);
    case '&':
        if (match(lx, '&')) return make_token(lx, TOK_AND, start);
        break;
    case '|':
        if (match(lx, '|')) return make_token(lx, TOK_OR, start);
        break;
    }
    return make_token(lx, TOK_ERROR, start);
}
