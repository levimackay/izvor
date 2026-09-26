#include <assert.h>
#include <stdio.h>
#include "../src/diag.h"

static void at(const char *src, long offset, int want_line, int want_col) {
    int line;
    int col;
    diag_line_col(src, offset, &line, &col);
    assert(line == want_line);
    assert(col == want_col);
}

int main(void) {
    at("abc", 0, 1, 1);
    at("abc", 1, 1, 2);
    at("abc", 2, 1, 3);
    at("abc", 3, 1, 4);
    at("abc", 99, 1, 4);
    at("abc", -5, 1, 1);

    at("ab\ncd", 2, 1, 3);
    at("ab\ncd", 3, 2, 1);
    at("ab\ncd", 4, 2, 2);

    at("a\n\nb", 2, 2, 1);
    at("a\n\nb", 3, 3, 1);

    at("\n\n\n", 3, 4, 1);
    at("", 0, 1, 1);
    at("\tx", 1, 1, 2);

    printf("test_diag_1 passed\n");
    return 0;
}
