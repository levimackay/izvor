/* Reference solution — task 0.6 */

void my_strcpy(char *dst, const char *src) {
    int i = 0;
    do {
        dst[i] = src[i];
    } while (src[i++] != '\0');  /* copies the terminator too */
}

int my_strcmp(const char *a, const char *b) {
    while (*a != '\0' && *a == *b) {
        a++;
        b++;
    }
    /* at the first difference (or the end of a): the char difference
     * is negative/zero/positive exactly as required */
    return (unsigned char)*a - (unsigned char)*b;
}

long parse_uint(const char *s) {
    long value = 0;
    while (*s >= '0' && *s <= '9') {
        value = value * 10 + (*s - '0');
        s++;
    }
    return value;
}

void reverse_in_place(char *s) {
    if (*s == '\0') return;      /* empty: nothing to do (and end-1 below
                                    would point before the array — UB) */
    char *end = s;
    while (*end != '\0') end++;  /* find the terminator... */
    end--;                       /* ...then step back to the last char */
    while (s < end) {
        char tmp = *s;
        *s = *end;
        *end = tmp;
        s++;
        end--;
    }
}
