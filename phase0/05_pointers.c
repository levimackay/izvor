/* Task 0.5 — Pointers I: addresses and dereferencing.
 * The model: a pointer is a variable whose VALUE is an ADDRESS.
 *   &x  — "where does x live?"        (address-of)
 *   *p  — "what lives at p?"          (dereference)
 * Build + run:   make p0-05
 */

/* Swap the values that a and b point at.
 * This is WHY pointers exist: without them a function can only touch
 * copies of its arguments. */
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

/* Write a + b into the location out points at.
 * This "out-parameter" pattern is everywhere in C — it's how a function
 * returns a second result. */
void store_sum(int a, int b, int *out) {
    *out = a + b;
}

/* Return whichever pointer points at the larger value (either on a tie).
 * Note: you return the POINTER, not the value. */
int *larger(int *a, int *b) {
    if (*a < *b) {
        return b;
    } else {
        return a;
    }

}

/* Add n to every element of the array through pointer arithmetic:
 * a + i is the address of element i, so *(a + i) is element i.
 * (a[i] is literally defined as *(a + i) — same thing, nicer clothes.) */
void add_to_all(int *a, int count, int n) {
    for (int i = 0; i < count; i++) {
        a[i] += n;
    }
}
