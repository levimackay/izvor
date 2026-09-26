#ifndef IZVOR_LEXER_H
#define IZVOR_LEXER_H

typedef enum {
    TOK_NUMBER,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_PERCENT,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_COMMA,
    TOK_COLON,
    TOK_SEMICOLON,
    TOK_ARROW,
    TOK_EQUAL,
    TOK_EQUAL_EQUAL,
    TOK_BANG,
    TOK_BANG_EQUAL,
    TOK_LESS,
    TOK_LESS_EQUAL,
    TOK_GREATER,
    TOK_GREATER_EQUAL,
    TOK_AND,
    TOK_OR,
    TOK_IDENT,
    TOK_PRINT,
    TOK_LET,
    TOK_VAR,
    TOK_FN,
    TOK_RETURN,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_TRUE,
    TOK_FALSE,
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
