/* Tests for the diagnostics module: offset to line and column, and the
   edges that a naive loop gets wrong. The rendered caret block is pinned
   separately by the golden tests in tests/golden. */

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
    /* single line: column counts from one, not from zero */
    at("abc", 0, 1, 1);
    at("abc", 1, 1, 2);
    at("abc", 2, 1, 3);

    /* the offset one past the end is where TOK_EOF lives, so it has to
       land somewhere sensible rather than run off the buffer */
    at("abc", 3, 1, 4);

    /* and anything further past the end clamps to that same spot */
    at("abc", 99, 1, 4);
    at("abc", -5, 1, 1);

    /* the newline itself belongs to the line it ends... */
    at("ab\ncd", 2, 1, 3);
    /* ...and the character after it starts the next line at column one */
    at("ab\ncd", 3, 2, 1);
    at("ab\ncd", 4, 2, 2);

    /* an empty line in the middle still advances the count */
    at("a\n\nb", 2, 2, 1);
    at("a\n\nb", 3, 3, 1);

    /* a file that is nothing but newlines */
    at("\n\n\n", 3, 4, 1);

    /* the empty source: one line, one column, no crash */
    at("", 0, 1, 1);

    /* a tab is one column wide, which is what lets the caret line up
       when diag_error prints the tab back out as a single space */
    at("\tx", 1, 1, 2);

    printf("test_diag_1 passed\n");
    return 0;
}
