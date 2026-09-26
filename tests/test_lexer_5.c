#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/lexer.h"

static void expect_types(const char *src, const TokenType *types) {
    Lexer lx;
    lexer_init(&lx, src);
    for (int i = 0;; i++) {
        Token t = lexer_next(&lx);
        assert(t.type == types[i]);
        if (t.type == TOK_EOF) break;
    }
}

int main(void) {
    Lexer lx;
    Token t;

    TokenType keywords[] = {
        TOK_LET, TOK_VAR, TOK_FN, TOK_RETURN, TOK_IF, TOK_ELSE, TOK_WHILE,
        TOK_PRINT, TOK_TRUE, TOK_FALSE, TOK_EOF
    };
    expect_types("let var fn return if else while print true false", keywords);

    TokenType idents[] = {TOK_IDENT, TOK_IDENT, TOK_IDENT, TOK_IDENT, TOK_IDENT, TOK_EOF};
    expect_types("letter lets _x x2 Int", idents);

    lexer_init(&lx, "letter");
    t = lexer_next(&lx);
    assert(t.type == TOK_IDENT && t.length == 6);

    TokenType pairs[] = {
        TOK_EQUAL_EQUAL, TOK_EQUAL, TOK_BANG_EQUAL, TOK_BANG, TOK_LESS_EQUAL,
        TOK_LESS, TOK_GREATER_EQUAL, TOK_GREATER, TOK_ARROW, TOK_MINUS,
        TOK_AND, TOK_OR, TOK_EOF
    };
    expect_types("== = != ! <= < >= > -> - && ||", pairs);

    TokenType punctuation[] = {
        TOK_LBRACE, TOK_RBRACE, TOK_COMMA, TOK_COLON, TOK_SEMICOLON, TOK_PERCENT, TOK_EOF
    };
    expect_types("{},:;%", punctuation);

    TokenType lone[] = {TOK_ERROR, TOK_ERROR, TOK_EOF};
    expect_types("& |", lone);

    TokenType commented[] = {TOK_NUMBER, TOK_NUMBER, TOK_EOF};
    expect_types("1 // 2 + 3 \xe2\x88\x86\n4 //", commented);

    lexer_init(&lx, "9223372036854775807");
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 9223372036854775807L);

    const char *big = "9223372036854775808 1";
    lexer_init(&lx, big);
    t = lexer_next(&lx);
    assert(t.type == TOK_ERROR);
    assert(t.start == big && t.length == 19);
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 1);

    assert(strcmp(token_type_name(TOK_ARROW), "ARROW") == 0);
    assert(strcmp(token_type_name(TOK_IDENT), "IDENT") == 0);

    printf("test_lexer_5 passed\n");
    return 0;
}
