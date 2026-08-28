/* Reference solution — task 0.8 */
#include <stdlib.h>
#include "08_dynarray.h"

void array_init(IntArray *a) {
    a->data = NULL;
    a->count = 0;
    a->capacity = 0;
}

void array_push(IntArray *a, int value) {
    if (a->count == a->capacity) {
        int new_capacity = (a->capacity < 8) ? 8 : a->capacity * 2;
        /* realloc(NULL, n) acts like malloc(n), so the first push needs
         * no special case */
        a->data = realloc(a->data, new_capacity * sizeof(int));
        a->capacity = new_capacity;
    }
    a->data[a->count] = value;
    a->count++;
}

int array_get(const IntArray *a, int i) {
    return a->data[i];
}

void array_free(IntArray *a) {
    free(a->data);
    /* reset to the empty state so the array is safe to reuse (and a
     * second array_free would be a harmless free(NULL)) */
    a->data = NULL;
    a->count = 0;
    a->capacity = 0;
}
