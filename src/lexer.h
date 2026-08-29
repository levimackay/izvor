#ifndef TINYLANG_LEXER_H
#define TINYLANG_LEXER_H

typedef enum {
    TOK_NUMBER,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_EOF,
    TOK_ERROR
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    int length;
    long value;
} Token;

typedef struct {
    const char *src;
    int pos;
} Lexer;

const char *token_type_name(TokenType type);
void lexer_init(Lexer *lx, const char *src);
Token lexer_next(Lexer *lx);

#endif
