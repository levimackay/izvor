/* Test for task 0.3 — do not edit; make these pass. */
#include <assert.h>
#include <stdio.h>

int max3(int a, int b, int c);
long factorial(int n);
int gcd(int a, int b);
int count_digits(int n);

int main(void) {
    assert(max3(1, 2, 3) == 3);
    assert(max3(3, 2, 1) == 3);
    assert(max3(2, 5, 2) == 5);
    assert(max3(-1, -2, -3) == -1);

    assert(factorial(0) == 1);
    assert(factorial(1) == 1);
    assert(factorial(5) == 120);
    assert(factorial(12) == 479001600L);

    assert(gcd(12, 18) == 6);
    assert(gcd(18, 12) == 6);
    assert(gcd(7, 13) == 1);
    assert(gcd(0, 5) == 5);

    assert(count_digits(0) == 1);
    assert(count_digits(7) == 1);
    assert(count_digits(42) == 2);
    assert(count_digits(100000) == 6);

    printf("task 0.3: all tests passed\n");
    return 0;
}
