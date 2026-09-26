#include <assert.h>
#include <stdio.h>
#include "../src/lexer.h"

int main(void) {
    Lexer lx;

    lexer_init(&lx, "");
    assert(lexer_next(&lx).type == TOK_EOF);
    assert(lexer_next(&lx).type == TOK_EOF);
    assert(lexer_next(&lx).type == TOK_EOF);

    const char *src = "+-*/()";
    lexer_init(&lx, src);

    Token t = lexer_next(&lx);
    assert(t.type == TOK_PLUS);
    assert(t.start == src);
    assert(t.length == 1);

    assert(lexer_next(&lx).type == TOK_MINUS);
    assert(lexer_next(&lx).type == TOK_STAR);
    assert(lexer_next(&lx).type == TOK_SLASH);

    t = lexer_next(&lx);
    assert(t.type == TOK_LPAREN);
    assert(t.start == src + 4);

    assert(lexer_next(&lx).type == TOK_RPAREN);
    assert(lexer_next(&lx).type == TOK_EOF);
    assert(lexer_next(&lx).type == TOK_EOF);

    lexer_init(&lx, "+");
    assert(lexer_next(&lx).type == TOK_PLUS);
    assert(lexer_next(&lx).type == TOK_EOF);

    printf("task 1.2: all tests passed\n");
    return 0;
}
