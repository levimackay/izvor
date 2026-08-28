/* Task 0.3 — Control flow and functions.
 * Loops and ifs only — no recursion yet, no library calls.
 * Build + run:   make p0-03
 */

/* Return the largest of a, b, c. */
int max3(int a, int b, int c) {
    (void)a; (void)b; (void)c;
    return 0; /* TODO */
}

/* Return n! computed with a loop (1 * 2 * ... * n). factorial(0) is 1.
 * Assume 0 <= n <= 12 (13! overflows int — try it later and see). */
long factorial(int n) {
    (void)n;
    return 0; /* TODO */
}

/* Greatest common divisor via Euclid's algorithm, with a loop:
 * while b isn't 0, replace (a, b) with (b, a % b); answer is a.
 * Assume a, b >= 0, not both zero. */
int gcd(int a, int b) {
    (void)a; (void)b;
    return 0; /* TODO */
}

/* How many decimal digits does n have? count_digits(0) is 1. Assume n >= 0.
 * (A lexer asks a cousin of this question: "how long is this number token?") */
int count_digits(int n) {
    (void)n;
    return 0; /* TODO */
}
