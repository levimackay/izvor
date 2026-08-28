/* Test for task 0.2 — do not edit; make these pass.
 * assert(x) aborts the program with a message if x is false. */
#include <assert.h>
#include <stdio.h>

/* Prototypes for the functions you implement in 02_variables.c.
 * In a bigger program these would live in a header file — that's task 0.9. */
int add(int a, int b);
double average(int a, int b);
double celsius_to_fahrenheit(double c);
int is_even(int n);
int last_digit(int n);

/* Doubles are inexact; never compare them with ==. */
static int close_to(double a, double b) {
    double d = a - b;
    return d < 0.000001 && d > -0.000001;
}

int main(void) {
    assert(add(2, 3) == 5);
    assert(add(-2, 2) == 0);

    assert(close_to(average(3, 4), 3.5));   /* int division would give 3.0 */
    assert(close_to(average(2, 2), 2.0));

    assert(close_to(celsius_to_fahrenheit(0.0), 32.0));
    assert(close_to(celsius_to_fahrenheit(100.0), 212.0));
    assert(close_to(celsius_to_fahrenheit(-40.0), -40.0));

    assert(is_even(4) == 1);
    assert(is_even(7) == 0);
    assert(is_even(0) == 1);

    assert(last_digit(1234) == 4);
    assert(last_digit(7) == 7);
    assert(last_digit(90) == 0);

    printf("task 0.2: all tests passed\n");
    return 0;
}
