/* lexdump: command-line tool that prints the token stream for
   the expression given as its argument. Development aid, not
   part of the compiler. */

#include <stdio.h>
#include "lexer.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <expression>\n", argv[0]);
        return 1;
    }

    Lexer lx;
    lexer_init(&lx, argv[1]);

    for (;;) {
        Token t = lexer_next(&lx);

        if (t.type == TOK_EOF) {
            printf("EOF\n");
            break;
        }

        printf("%-8s '%.*s'", token_type_name(t.type), t.length, t.start);
        if (t.type == TOK_NUMBER) printf("   value=%ld", t.value);
        printf("\n");
    }

    return 0;
}
