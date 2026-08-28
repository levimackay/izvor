/* Test for task 0.8 — do not edit; make these pass.
 * After it passes, check the other iron rule (every malloc freed) with
 * macOS's built-in leak detector:   leaks --atExit -- ./build/p0-08
 */
#include <assert.h>
#include <stdio.h>
#include "08_dynarray.h"

int main(void) {
    IntArray a;
    array_init(&a);
    assert(a.count == 0);
    assert(a.capacity == 0);
    assert(a.data == 0);

    array_push(&a, 42);
    assert(a.count == 1);
    assert(array_get(&a, 0) == 42);

    /* push enough to force several regrowths */
    for (int i = 1; i < 1000; i++) {
        array_push(&a, i * 2);
    }
    assert(a.count == 1000);
    assert(array_get(&a, 0) == 42);      /* survived every realloc */
    assert(array_get(&a, 1) == 2);
    assert(array_get(&a, 999) == 1998);
    assert(a.capacity >= a.count);

    array_free(&a);
    assert(a.count == 0 && a.capacity == 0 && a.data == 0);

    /* the array must be reusable after free */
    array_push(&a, 7);
    assert(a.count == 1 && array_get(&a, 0) == 7);
    array_free(&a);

    printf("task 0.8: all tests passed\n");
    return 0;
}
