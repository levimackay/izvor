#include <assert.h>
#include <stdio.h>
#include "../src/lexer.h"

int main(void) {
    Lexer lx;
    Token t;

    const char *src = "123";
    lexer_init(&lx, src);
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER);
    assert(t.value == 123);
    assert(t.start == src);
    assert(t.length == 3);
    assert(lexer_next(&lx).type == TOK_EOF);

    lexer_init(&lx, "7");
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 7 && t.length == 1);
    lexer_init(&lx, "0");
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 0);

    lexer_init(&lx, "12+34");
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 12 && t.length == 2);
    assert(lexer_next(&lx).type == TOK_PLUS);
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 34);
    assert(lexer_next(&lx).type == TOK_EOF);

    lexer_init(&lx, "  12 \t+\n 34  ");
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 12);
    assert(t.length == 2);
    assert(lexer_next(&lx).type == TOK_PLUS);
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 34);
    assert(lexer_next(&lx).type == TOK_EOF);

    lexer_init(&lx, "   \n\t ");
    assert(lexer_next(&lx).type == TOK_EOF);

    printf("task 1.3: all tests passed\n");
    return 0;
}
