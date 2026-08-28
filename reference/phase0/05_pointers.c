/* Reference solution — task 0.5 */

void swap(int *a, int *b) {
    int tmp = *a;   /* read what a points at */
    *a = *b;        /* works even when a == b: we saved tmp first */
    *b = tmp;
}

void store_sum(int a, int b, int *out) {
    *out = a + b;
}

int *larger(int *a, int *b) {
    return (*a >= *b) ? a : b;   /* compare VALUES, return the POINTER */
}

void add_to_all(int *a, int count, int n) {
    for (int i = 0; i < count; i++) {
        *(a + i) += n;           /* identical to: a[i] += n */
    }
}
