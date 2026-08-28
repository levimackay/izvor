/* Task 0.2 — Variables, types, arithmetic.
 *
 * Implement each function below. Delete the `(void)` lines (they only
 * silence "unused parameter" warnings while the function is a stub) and
 * the placeholder return.
 *
 * Build + run the test:   make p0-02
 * Doc:                    docs/phases/00-c-fundamentals.md
 */

/* Return a + b. (Warm-up: just make the build/test loop work.) */
int add(int a, int b) {
    (void)a; (void)b;
    return 0; /* TODO */
}

/* Return the average of a and b as a double.
 * Careful: (a + b) / 2 with ints THROWS AWAY the fraction before you ever
 * reach the return type. You need to make the division happen in double. */
double average(int a, int b) {
    (void)a; (void)b;
    return 0.0; /* TODO */
}

/* Convert Celsius to Fahrenheit: F = C * 9/5 + 32.
 * Same trap as average: 9/5 in int arithmetic is 1. */
double celsius_to_fahrenheit(double c) {
    (void)c;
    return 0.0; /* TODO */
}

/* Return 1 if n is even, 0 if odd. (C has no built-in bool in the way
 * you might expect — int 0/1 is the traditional currency. The modulo
 * operator % is your friend.) */
int is_even(int n) {
    (void)n;
    return 0; /* TODO */
}

/* Return the last decimal digit of n. Assume n >= 0.
 * (This is a tiny preview of lexing: numbers are made of digits, and
 * arithmetic can take them apart.) */
int last_digit(int n) {
    (void)n;
    return 0; /* TODO */
}
