/* Reference solution — task 0.9 (same logic as 08_dynarray.c, now living
 * behind the intarray.h interface) */
#include <stdlib.h>
#include "intarray.h"

void array_init(IntArray *a) {
    a->data = NULL;
    a->count = 0;
    a->capacity = 0;
}

void array_push(IntArray *a, int value) {
    if (a->count == a->capacity) {
        int new_capacity = (a->capacity < 8) ? 8 : a->capacity * 2;
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
    a->data = NULL;
    a->count = 0;
    a->capacity = 0;
}
