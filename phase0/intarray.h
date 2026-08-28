/* Task 0.9 — the IntArray as a proper two-file module.
 *
 * A header is a PROMISE: "these types and functions exist somewhere".
 * Any .c file that includes it can use them; the linker later finds the
 * actual definitions (in intarray.o) and stitches the calls up.
 *
 * The #ifndef/#define pair is an INCLUDE GUARD: without it, a file that
 * ends up including this header twice (which happens constantly through
 * chains of includes) would see the struct defined twice — an error.
 */
#ifndef TINYLANG_PHASE0_INTARRAY_H
#define TINYLANG_PHASE0_INTARRAY_H

typedef struct {
    int *data;
    int count;
    int capacity;
} IntArray;

void array_init(IntArray *a);
void array_push(IntArray *a, int value);
int  array_get(const IntArray *a, int i);
void array_free(IntArray *a);

#endif
