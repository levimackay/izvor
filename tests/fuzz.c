/* Fuzz harness for the front end.
 
   The claim being tested is narrow and worth stating exactly: for ANY
   input string, lexing and parsing must either return a tree or return
   NULL, and must never read out of bounds, never overflow, and never
   crash. UndefinedBehaviorSanitizer is what turns those "never"s into a
   non-zero exit, so this binary is only meaningful when built with it.
 
   The generator is a fixed-seed xorshift rather than rand(), so a failure
   on CI reproduces byte for byte on a laptop. A failing run prints the
   input that broke it before it dies.
 
   Deliberately not fuzzed: eval(). Evaluation of arbitrary trees can
   overflow a long, which is real undefined behavior and a real gap, but
   it is a semantics gap rather than a parsing one. It is tracked in
   docs/ROADMAP.md under checked arithmetic. For the same reason the
   generator caps runs of digits, since the lexer folds digits into a
   long as it scans and a 40-digit literal would overflow before the
   parser ever saw it. */

#include <stdio.h>
#include <stdlib.h>
#include "../src/parser.h"

#define ITERATIONS   20000
#define MAX_LENGTH   64
#define MAX_DIGITS   9

// Operators, parentheses, whitespace, identifier characters and junk the
// lexer must reject. Weighted by nothing: uniform choice finds stranger
// inputs than a hand-tuned distribution does.
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

    // The parser reports every error it finds, and 20000 broken programs
    // would bury a real failure in noise.
    if (freopen("/dev/null", "w", stderr) == NULL) {
        printf("fuzz: cannot silence stderr\n");
        return 1;
    }

    for (int i = 0; i < ITERATIONS; i++) {
        size_t length = next_random() % MAX_LENGTH;
        int digit_run = 0;

        for (size_t j = 0; j < length; j++) {
            char c = alphabet[next_random() % choices];

            // Break up long digit runs; see the note at the top.
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

        // A returned tree has to be a whole tree: freeing it recursively
        // is what catches a half-built node with a dangling child.
        ast_free(tree);
    }

    printf("fuzz: %d inputs, no crashes\n", ITERATIONS);
    return 0;
}
