/* Diagnostics implementation: one error format for the whole compiler.
   The layout is the one rustc popularized, because it answers the three
   questions in order: what is wrong, where is it, and what does it look
   like. */

#include <stdarg.h>
#include <stdio.h>
#include "diag.h"

// The file name printed in the --> line. The parser never learns what
// file it is reading, so the driver leaves the name here instead of
// threading a path through every function that might report an error.
static const char *current_path = "<input>";

void diag_set_path(const char *path) {
    current_path = (path == NULL) ? "<input>" : path;
}

// Walk to the NUL to find the length, then pull the offset inside it.
// An offset one past the end is normal: that is where TOK_EOF lives.
static long clamp(const char *src, long offset) {
    long length = 0;
    while (src[length] != '\0') {
        length++;
    }
    if (offset < 0) {
        return 0;
    }
    if (offset > length) {
        return length;
    }
    return offset;
}

void diag_line_col(const char *src, long offset, int *line, int *col) {
    offset = clamp(src, offset);

    // Counting from the start on every error is O(n) per message. Errors
    // are rare and the source fits in memory, so a line table would be
    // machinery with nothing to pay for it.
    int l = 1;
    int c = 1;
    for (long i = 0; i < offset; i++) {
        if (src[i] == '\n') {
            l++;
            c = 1;
        } else {
            c++;
        }
    }

    *line = l;
    *col = c;
}

/* at_end_of_source — an error at the very end of a file lands on the NUL
   after the final newline, and a caret under a blank line tells nobody
   anything. Back up over the trailing newlines so it points just past
   the last line that has something on it. This is presentation only,
   which is why diag_line_col does not do it. */
static long at_end_of_source(const char *src, long offset) {
    if (src[offset] != '\0') {
        return offset;
    }
    while (offset > 0 && (src[offset - 1] == '\n' || src[offset - 1] == '\r')) {
        offset--;
    }
    return offset;
}

/* line_bounds — find where the line holding offset starts and how long
   it is, not counting the newline that ends it. */
static void line_bounds(const char *src, long offset, long *start, long *length) {
    long s = offset;
    while (s > 0 && src[s - 1] != '\n') {
        s--;
    }
    long e = offset;
    while (src[e] != '\0' && src[e] != '\n') {
        e++;
    }
    *start = s;
    *length = e - s;
}

void diag_error(const char *src, long offset, const char *fmt, ...) {
    offset = at_end_of_source(src, clamp(src, offset));

    int line;
    int col;
    diag_line_col(src, offset, &line, &col);

    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    va_end(args);

    // The gutter is exactly as wide as the line number, so the bars stay
    // lined up whether the error is on line 3 or line 3000.
    char number[16];
    int width = snprintf(number, sizeof number, "%d", line);

    fprintf(stderr, "%*s--> %s:%d:%d\n", width, "", current_path, line, col);
    fprintf(stderr, "%*s|\n", width + 1, "");

    long start;
    long length;
    line_bounds(src, offset, &start, &length);

    fprintf(stderr, "%s | ", number);
    for (long i = 0; i < length; i++) {
        // A tab printed as a tab would shove the caret out of line, so it
        // goes out as one space and one byte stays worth one column.
        char c = src[start + i];
        fputc(c == '\t' ? ' ' : c, stderr);
    }
    fputc('\n', stderr);

    fprintf(stderr, "%*s| %*s^\n", width + 1, "", col - 1, "");
}
