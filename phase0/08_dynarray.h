/* Task 0.8 — a growable array of ints.
 * THE data structure of this whole project: your token list, your
 * bytecode, and your constant pool will all be this shape. */
#ifndef PHASE0_08_DYNARRAY_H
#define PHASE0_08_DYNARRAY_H

typedef struct {
    int *data;      /* heap buffer from malloc/realloc (NULL when empty) */
    int count;      /* how many elements are in use */
    int capacity;   /* how many elements the buffer can hold */
} IntArray;

void array_init(IntArray *a);              /* empty: NULL/0/0 — no allocation yet */
void array_push(IntArray *a, int value);   /* append, growing if full */
int  array_get(const IntArray *a, int i);  /* element i (0 <= i < count) */
void array_free(IntArray *a);              /* free the buffer, back to empty state */

#endif
