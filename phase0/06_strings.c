/* Task 0.6 — Pointers II: walking strings. Your first micro-lexer.
 * Still no <string.h>.
 * Build + run:   make p0-06
 */

/* Copy src (including its '\0') into dst. Assume dst is big enough.
 * The classic idiom is a loop copying until you've copied the terminator. */
void my_strcpy(char *dst, const char *src) {
    int i = 0;

    while (src[i] != '\0'){
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

/* Compare like the real strcmp: walk both strings together; at the first
 * position where they differ (or one ends), return negative if a < b,
 * 0 if equal, positive if a > b (comparing the chars as numbers). */
int my_strcmp(const char *a, const char *b) {
    int i = 0;
    
    while (a[i] != '\0' && a[i] == b[i]) {
        i++;
    }
    return a[i] - b[i];
}

/* Parse the leading decimal digits of s into a number:
 *   parse_uint("123abc") == 123     parse_uint("abc") == 0
 * The accumulation trick: value = value * 10 + (c - '0').
 * ('0'..'9' are consecutive character codes, so c - '0' turns the
 * CHARACTER '7' into the NUMBER 7.)
 *
 * This function IS a piece of your Phase 1 lexer. */
long parse_uint(const char *s) {
    int i = 0;
    long value = 0;

    while (s[i] != '\0' && s[i] >= '0' && s[i] <= '9'){
        value = value *10+(s[i]-'0');
        i++;
    }
    return value;
}

/* Reverse s in place (first char swaps with last, and so on inward).
 * Two pointers walking toward each other is the classic shape. */
void reverse_in_place(char *s) {
    int count = 0;

    while (s[count] != '\0'){
        count++;
    }

    int left = 0;
    int right = count -1;

    while (left < right){
        char temp = s[left];
        s[left] = s[right];
        s[right] = temp;
        left++;
        right--;
    }
    
}
