/* Task 0.8 — malloc/free and the growable array.
 *
 * The growth strategy (used by nearly every dynamic array ever shipped):
 *   - if count == capacity, the buffer is full: pick a new capacity
 *     (8 for the first allocation, double it after), realloc the buffer,
 *     and update capacity.
 *   - then store the value at index count and increment count.
 *
 * Iron rules, enforced by the sanitizer and the `leaks` tool:
 *   - every malloc/realloc'd buffer is freed exactly once
 *   - never touch a buffer after freeing it
 *   - array_free must leave the struct safe to array_init or free again
 *
 * Build + run:   make p0-08
 * Then check for leaks:   leaks --atExit -- ./build/p0-08
 */
#include <stdlib.h>   /* malloc, realloc, free */
#include "08_dynarray.h"

void array_init(IntArray *a) {
    (void)a;
    /* TODO */
}

void array_push(IntArray *a, int value) {
    (void)a; (void)value;
    /* TODO: grow if full (realloc handles the "first time" case too —
     * realloc(NULL, n) behaves like malloc(n)), then append. */
}

int array_get(const IntArray *a, int i) {
    (void)a; (void)i;
    return 0; /* TODO */
}

void array_free(IntArray *a) {
    (void)a;
    /* TODO: free the buffer AND reset the fields to the empty state. */
}
