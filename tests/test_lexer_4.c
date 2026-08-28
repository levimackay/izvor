/* Test for task 1.4 — error tokens + a full expression sweep. Do not edit. */
#include <assert.h>
#include <stdio.h>
#include "../src/lexer.h"

int main(void) {
    Lexer lx;
    Token t;

    /* an unrecognized character becomes an ERROR token... */
    const char *src = "1 @ 2";
    lexer_init(&lx, src);
    assert(lexer_next(&lx).type == TOK_NUMBER);
    t = lexer_next(&lx);
    assert(t.type == TOK_ERROR);
    assert(t.start == src + 2);        /* points at the '@' */
    assert(t.length == 1);
    /* ...and the lexer keeps going afterwards */
    t = lexer_next(&lx);
    assert(t.type == TOK_NUMBER && t.value == 2);
    assert(lexer_next(&lx).type == TOK_EOF);

    /* several bad characters: one error each, no crash */
    lexer_init(&lx, "@$");
    assert(lexer_next(&lx).type == TOK_ERROR);
    assert(lexer_next(&lx).type == TOK_ERROR);
    assert(lexer_next(&lx).type == TOK_EOF);

    /* the full sweep: (1 + 23) * 4 - 5 / 2 */
    lexer_init(&lx, "(1 + 23) * 4 - 5 / 2");
    TokenType expected[] = {
        TOK_LPAREN, TOK_NUMBER, TOK_PLUS, TOK_NUMBER, TOK_RPAREN,
        TOK_STAR, TOK_NUMBER, TOK_MINUS, TOK_NUMBER, TOK_SLASH,
        TOK_NUMBER, TOK_EOF
    };
    long expected_values[] = {0, 1, 0, 23, 0, 0, 4, 0, 5, 0, 2, 0};
    int n = (int)(sizeof(expected) / sizeof(expected[0]));
    for (int i = 0; i < n; i++) {
        t = lexer_next(&lx);
        assert(t.type == expected[i]);
        if (t.type == TOK_NUMBER) {
            assert(t.value == expected_values[i]);
        }
    }

    printf("task 1.4: all tests passed\n");
    return 0;
}
