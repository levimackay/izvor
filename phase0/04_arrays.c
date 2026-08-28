/* Task 0.4 — Arrays and strings.
 * No <string.h> allowed — the point is to build strlen yourself once.
 * Build + run:   make p0-04
 */

/* Sum of the first n elements of a. Note the signature: in C an array
 * arrives as a POINTER to its first element, and the length must travel
 * separately — the array itself doesn't know how long it is. */
int sum_array(const int *a, int n) {
    int total = 0;
    for (int i = 0; i < n; i++){
        total += a[i];
    }
    return total;
}

/* Largest of the first n elements. Assume n >= 1. */
int max_in_array(const int *a, int n) {
    int m = a[0];
    for (int i = 1; i < n; i++) {
        if (a[i] > m) m = a[i];
    }
    return m;
}

/* Length of C string s: count characters until the '\0' terminator.
 * (Your whole language's source code will be one of these strings, and
 * the lexer is just a careful walk like this one.) */
int my_strlen(const char *s) {
    int i = 0;
    while (s[i] != '\0'){
        i++;
    }
    return i;
}

/* How many times does character c appear in s? */
int count_char(const char *s, char c) {
    int count = 0;
    int i = 0;

    while (s[i] != '\0'){
        if (s[i] == c) count++;
        i++;
    }

    return count;
}
