/* Reference solution — task 1.5, lexdump. */
#include <stdio.h>
#include "lexer.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s \"expression\"\n", argv[0]);
        return 1;
    }

    Lexer lx;
    lexer_init(&lx, argv[1]);

    for (;;) {
        Token t = lexer_next(&lx);
        /* %.*s prints exactly t.length chars starting at t.start —
         * the token doesn't own a NUL-terminated copy of its text */
        printf("%-9s '%.*s'", token_type_name(t.type), t.length, t.start);
        if (t.type == TOK_NUMBER) {
            printf("   value=%ld", t.value);
        }
        printf("\n");
        if (t.type == TOK_EOF) {
            return 0;
        }
    }
}
