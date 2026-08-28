/* Task 1.5 — lexdump: print the token stream for an expression.
 *
 *   make lexdump
 *   ./build/lexdump "12 + 3*(40-5)"
 *
 * Expected output shape (exact formatting is your call):
 *
 *   NUMBER    '12'   value=12
 *   PLUS      '+'
 *   ...
 *   EOF
 *
 * TODO:
 *   - if argc != 2, print a usage line to stderr and return 1
 *     (argv[0] is the program name; argv[1] is your expression)
 *   - init a lexer on argv[1]
 *   - loop: token = lexer_next(...); print it; stop after TOK_EOF
 *   - printing the lexeme: token.start is NOT NUL-terminated at the
 *     right place — print exactly token.length chars with the printf
 *     precision trick:   printf("%.*s", token.length, token.start);
 */
#include <stdio.h>
#include "lexer.h"

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    /* TODO */
    return 0;
}
