#include <stdarg.h>
#include <stdio.h>
#include "diag.h"

static const char *current_path = "<input>";

void diag_set_path(const char *path) {
    current_path = (path == NULL) ? "<input>" : path;
}

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

static long at_end_of_source(const char *src, long offset) {
    if (src[offset] != '\0') {
        return offset;
    }
    while (offset > 0 && (src[offset - 1] == '\n' || src[offset - 1] == '\r')) {
        offset--;
    }
    return offset;
}

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

    char number[16];
    int width = snprintf(number, sizeof number, "%d", line);

    fprintf(stderr, "%*s--> %s:%d:%d\n", width, "", current_path, line, col);
    fprintf(stderr, "%*s|\n", width + 1, "");

    long start;
    long length;
    line_bounds(src, offset, &start, &length);

    fprintf(stderr, "%s | ", number);
    for (long i = 0; i < length; i++) {
        char c = src[start + i];
        fputc(c == '\t' ? ' ' : c, stderr);
    }
    fputc('\n', stderr);

    fprintf(stderr, "%*s| %*s^\n", width + 1, "", col - 1, "");
}
