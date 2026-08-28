/* Task 0.3 — Control flow and functions.
 * Loops and ifs only — no recursion yet, no library calls.
 * Build + run:   make p0-03
 */

/* Return the largest of a, b, c. */
int max3(int a, int b, int c) {
    int m = a;
    if (b > m) m = b;
    if (c > m) m = c;
    return m;
}


/* Return n! computed with a loop (1 * 2 * ... * n). factorial(0) is 1.
 * Assume 0 <= n <= 12 (13! overflows int — try it later and see). */
long factorial(int n) {
    long result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

/* Greatest common divisor via Euclid's algorithm, with a loop:
 * while b isn't 0, replace (a, b) with (b, a % b); answer is a.
 * Assume a, b >= 0, not both zero. */
int gcd(int a, int b) {
    while (b != 0){
        int temp = a;
        a = b;
        b = temp % b;
    }
    return a;
}

/* How many decimal digits does n have? count_digits(0) is 1. Assume n >= 0.
 * (A lexer asks a cousin of this question: "how long is this number token?") */
int count_digits(int n) {
    int count = 0;
    if (n == 0) return 1;
    while (n != 0){
        n = n / 10;
        count++;
    }
    return count;
}
