#include <stdio.h>
#include <stdlib.h>
#include "../src/parser.h"

#define ITERATIONS   20000
#define MAX_LENGTH   64
#define MAX_DIGITS   9

static const char alphabet[] = "0123456789+-*/() \t\nxyz_letvarprint=@$\"'";

static unsigned int state = 0x9E3779B9u;

static unsigned int next_random(void) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

int main(void) {
    char buffer[MAX_LENGTH + 1];
    const size_t choices = sizeof alphabet - 1;

    if (freopen("/dev/null", "w", stderr) == NULL) {
        printf("fuzz: cannot silence stderr\n");
        return 1;
    }

    for (int i = 0; i < ITERATIONS; i++) {
        size_t length = next_random() % MAX_LENGTH;
        int digit_run = 0;

        for (size_t j = 0; j < length; j++) {
            char c = alphabet[next_random() % choices];

            if (c >= '0' && c <= '9') {
                if (digit_run >= MAX_DIGITS) {
                    c = ' ';
                    digit_run = 0;
                } else {
                    digit_run++;
                }
            } else {
                digit_run = 0;
            }

            buffer[j] = c;
        }
        buffer[length] = '\0';

        Parser p;
        parser_init(&p, buffer);
        Node *tree = parser_parse(&p);

        ast_free(tree);
    }

    printf("fuzz: %d inputs, no crashes\n", ITERATIONS);
    return 0;
}
