/* Test for task 0.4 — do not edit; make these pass. */
#include <assert.h>
#include <stdio.h>

int sum_array(const int *a, int n);
int max_in_array(const int *a, int n);
int my_strlen(const char *s);
int count_char(const char *s, char c);

int main(void) {
    int a[] = {3, 1, 4, 1, 5};
    assert(sum_array(a, 5) == 14);
    assert(sum_array(a, 1) == 3);
    assert(sum_array(a, 0) == 0);

    assert(max_in_array(a, 5) == 5);
    assert(max_in_array(a, 3) == 4);
    int b[] = {-5, -2, -9};
    assert(max_in_array(b, 3) == -2);

    assert(my_strlen("") == 0);
    assert(my_strlen("a") == 1);
    assert(my_strlen("hello, world") == 12);

    assert(count_char("banana", 'a') == 3);
    assert(count_char("banana", 'z') == 0);
    assert(count_char("", 'a') == 0);

    printf("task 0.4: all tests passed\n");
    return 0;
}
