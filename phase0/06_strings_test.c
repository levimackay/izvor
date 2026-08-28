/* Test for task 0.6 — do not edit; make these pass. */
#include <assert.h>
#include <stdio.h>

void my_strcpy(char *dst, const char *src);
int my_strcmp(const char *a, const char *b);
long parse_uint(const char *s);
void reverse_in_place(char *s);

int main(void) {
    char buf[32];
    my_strcpy(buf, "hello");
    assert(my_strcmp(buf, "hello") == 0);
    my_strcpy(buf, "");
    assert(buf[0] == '\0');

    assert(my_strcmp("abc", "abc") == 0);
    assert(my_strcmp("abc", "abd") < 0);
    assert(my_strcmp("abd", "abc") > 0);
    assert(my_strcmp("ab", "abc") < 0);   /* prefix is "smaller" */
    assert(my_strcmp("abc", "ab") > 0);
    assert(my_strcmp("", "") == 0);

    assert(parse_uint("0") == 0);
    assert(parse_uint("7") == 7);
    assert(parse_uint("123abc") == 123);
    assert(parse_uint("abc") == 0);
    assert(parse_uint("00042") == 42);

    char s1[] = "abcd";
    reverse_in_place(s1);
    assert(my_strcmp(s1, "dcba") == 0);
    char s2[] = "abc";                    /* odd length */
    reverse_in_place(s2);
    assert(my_strcmp(s2, "cba") == 0);
    char s3[] = "";                       /* empty must not crash */
    reverse_in_place(s3);
    assert(s3[0] == '\0');

    printf("task 0.6: all tests passed\n");
    return 0;
}
