/* Task 0.4 — Arrays and strings.
 * No <string.h> allowed — the point is to build strlen yourself once.
 * Build + run:   make p0-04
 */

/* Sum of the first n elements of a. Note the signature: in C an array
 * arrives as a POINTER to its first element, and the length must travel
 * separately — the array itself doesn't know how long it is. */
int sum_array(const int *a, int n) {
    (void)a; (void)n;
    return 0; /* TODO */
}

/* Largest of the first n elements. Assume n >= 1. */
int max_in_array(const int *a, int n) {
    (void)a; (void)n;
    return 0; /* TODO */
}

/* Length of C string s: count characters until the '\0' terminator.
 * (Your whole language's source code will be one of these strings, and
 * the lexer is just a careful walk like this one.) */
int my_strlen(const char *s) {
    (void)s;
    return 0; /* TODO */
}

/* How many times does character c appear in s? */
int count_char(const char *s, char c) {
    (void)s; (void)c;
    return 0; /* TODO */
}
