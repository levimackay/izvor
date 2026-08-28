/* Test for task 1.3 — whitespace + number literals. Do not edit. */
#include <assert.h>
#include <stdio.h>
#include "../src/lexer.h"

int main(void) {
    Lexer lx;
    Token t;

    /* a single number */
    const char *src = "123";
    lexer_init(&lx, src);
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER);
    assert(t.value == 123);
    assert(t.start == src);
    assert(t.length == 3);
    assert(lexer_next(&lx).type == TOK_EOF);

    /* single digit, and zero */
    lexer_init(&lx, "7");
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 7 && t.length == 1);
    lexer_init(&lx, "0");
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 0);

    /* numbers end where non-digits begin */
    lexer_init(&lx, "12+34");
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 12 && t.length == 2);
    assert(lexer_next(&lx).type == TOK_PLUS);
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 34);
    assert(lexer_next(&lx).type == TOK_EOF);

    /* whitespace (spaces, tabs, newlines) is invisible */
    lexer_init(&lx, "  12 \t+\n 34  ");
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 12);
    assert(t.length == 2);              /* lexeme excludes the spaces */
    assert(lexer_next(&lx).type == TOK_PLUS);
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 34);
    assert(lexer_next(&lx).type == TOK_EOF);

    /* whitespace-only source is just EOF */
    lexer_init(&lx, "   \n\t ");
    assert(lexer_next(&lx).type == TOK_EOF);

    printf("task 1.3: all tests passed\n");
    return 0;
}
