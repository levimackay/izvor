#include <stdio.h>
#include <stdlib.h>
#include "../src/check.h"
#include "../src/parser.h"

#define ITERATIONS 20000
#define MAX_LENGTH 96

static const char *const pieces[] = {
    "0", "1", "42", "9223372036854775807", "99999999999999999999",
    "+", "-", "*", "/", "%", "(", ")", "{", "}", ",", ":", ";", "->",
    "=", "==", "!", "!=", "<", "<=", ">", ">=", "&&", "||", "&", "|",
    "let ", "var ", "fn ", "return ", "if ", "else ", "while ", "print",
    "true", "false", "Int", "Bool", "x", "y", "f", "_n2",
    " ", "\t", "\n", "//", "@", "$", "\"", "\xe2\x88\x86",
};

static unsigned int state = 0x9E3779B9u;

static unsigned int next_random(void) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

int main(void) {
    char buffer[MAX_LENGTH * 20 + 1];
    const size_t choices = sizeof pieces / sizeof pieces[0];
    int checked = 0;

    if (freopen("/dev/null", "w", stderr) == NULL) {
        printf("fuzz: cannot silence stderr\n");
        return 1;
    }

    for (int i = 0; i < ITERATIONS; i++) {
        size_t count = next_random() % MAX_LENGTH;
        size_t length = 0;
        for (size_t j = 0; j < count; j++) {
            const char *piece = pieces[next_random() % choices];
            while (*piece != '\0') {
                buffer[length++] = *piece++;
            }
        }
        buffer[length] = '\0';

        Parser p;
        parser_init(&p, buffer);
        Node *expr = parser_parse(&p);
        if (expr != NULL) {
            check_expression(buffer, expr);
        }
        ast_free(expr);

        parser_init(&p, buffer);
        Program prog = parser_parse_program(&p);
        if (!p.had_error && check_program(buffer, &prog)) {
            checked++;
        }
        program_free(&prog);
    }

    printf("fuzz: %d inputs, %d type checked, no crashes\n", ITERATIONS, checked);
    return 0;
}
