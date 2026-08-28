/* Test for task 0.5 — do not edit; make these pass. */
#include <assert.h>
#include <stdio.h>

void swap(int *a, int *b);
void store_sum(int a, int b, int *out);
int *larger(int *a, int *b);
void add_to_all(int *a, int count, int n);

int main(void) {
    int x = 1, y = 2;
    swap(&x, &y);
    assert(x == 2 && y == 1);
    swap(&x, &x);            /* swapping with itself must not zero it */
    assert(x == 2);

    int out = 0;
    store_sum(3, 4, &out);
    assert(out == 7);

    int a = 10, b = 20;
    assert(larger(&a, &b) == &b);   /* compares ADDRESSES: right pointer? */
    assert(*larger(&b, &a) == 20);
    assert(larger(&a, &a) == &a);

    int arr[] = {1, 2, 3};
    add_to_all(arr, 3, 10);
    assert(arr[0] == 11 && arr[1] == 12 && arr[2] == 13);
    add_to_all(arr, 0, 100);        /* zero elements: must touch nothing */
    assert(arr[0] == 11);

    printf("task 0.5: all tests passed\n");
    return 0;
}
