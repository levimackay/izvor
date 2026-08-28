/* Test for task 0.9 — do not edit; make these pass.
 * Same behavior as task 0.8, but note the difference in THIS file:
 * no hand-written prototypes — it includes the header instead. */
#include <assert.h>
#include <stdio.h>
#include "intarray.h"

int main(void) {
    IntArray a;
    array_init(&a);

    for (int i = 0; i < 100; i++) {
        array_push(&a, i * i);
    }
    assert(a.count == 100);
    assert(array_get(&a, 10) == 100);
    assert(array_get(&a, 99) == 9801);

    array_free(&a);
    assert(a.data == 0 && a.count == 0 && a.capacity == 0);

    printf("task 0.9: all tests passed\n");
    return 0;
}
