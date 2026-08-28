/* Test for task 1.1 — token_type_name. Do not edit; make it pass. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/lexer.h"

int main(void) {
    assert(strcmp(token_type_name(TOK_NUMBER), "NUMBER") == 0);
    assert(strcmp(token_type_name(TOK_PLUS),   "PLUS")   == 0);
    assert(strcmp(token_type_name(TOK_MINUS),  "MINUS")  == 0);
    assert(strcmp(token_type_name(TOK_STAR),   "STAR")   == 0);
    assert(strcmp(token_type_name(TOK_SLASH),  "SLASH")  == 0);
    assert(strcmp(token_type_name(TOK_LPAREN), "LPAREN") == 0);
    assert(strcmp(token_type_name(TOK_RPAREN), "RPAREN") == 0);
    assert(strcmp(token_type_name(TOK_EOF),    "EOF")    == 0);
    assert(strcmp(token_type_name(TOK_ERROR),  "ERROR")  == 0);

    printf("task 1.1: all tests passed\n");
    return 0;
}
