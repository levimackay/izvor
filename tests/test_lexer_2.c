/* Test for task 1.2 — init + single-character tokens. Do not edit. */
#include <assert.h>
#include <stdio.h>
#include "../src/lexer.h"

int main(void) {
    Lexer lx;

    /* empty source: EOF immediately, and EOF is sticky */
    lexer_init(&lx, "");
    assert(lexer_next(&lx).type == TOK_EOF);
    assert(lexer_next(&lx).type == TOK_EOF);
    assert(lexer_next(&lx).type == TOK_EOF);

    /* each single-character token, in order */
    const char *src = "+-*/()";
    lexer_init(&lx, src);

    Token t = lexer_next(&lx);
    assert(t.type == TOK_PLUS);
    assert(t.start == src);        /* points at the '+' in the source */
    assert(t.length == 1);

    assert(lexer_next(&lx).type == TOK_MINUS);
    assert(lexer_next(&lx).type == TOK_STAR);
    assert(lexer_next(&lx).type == TOK_SLASH);

    t = lexer_next(&lx);
    assert(t.type == TOK_LPAREN);
    assert(t.start == src + 4);

    assert(lexer_next(&lx).type == TOK_RPAREN);
    assert(lexer_next(&lx).type == TOK_EOF);
    assert(lexer_next(&lx).type == TOK_EOF);   /* still sticky */

    /* re-init rewinds */
    lexer_init(&lx, "+");
    assert(lexer_next(&lx).type == TOK_PLUS);
    assert(lexer_next(&lx).type == TOK_EOF);

    printf("task 1.2: all tests passed\n");
    return 0;
}
