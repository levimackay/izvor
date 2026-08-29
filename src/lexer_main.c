/* Task 1.5 — lexdump: print the token stream for an expression.
 *
 *   make lexdump
 *   ./build/lexdump "12 + 3*(40-5)"
 */
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
