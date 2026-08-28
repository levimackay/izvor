/* Reference solution — task 0.2 */

int add(int a, int b) {
    return a + b;
}

double average(int a, int b) {
    /* dividing by 2.0 (a double) promotes the whole expression to double
     * BEFORE the division, so the fraction survives */
    return (a + b) / 2.0;
}

double celsius_to_fahrenheit(double c) {
    return c * 9.0 / 5.0 + 32.0;
}

int is_even(int n) {
    return n % 2 == 0;
}

int last_digit(int n) {
    return n % 10;
}
