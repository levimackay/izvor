#ifndef IZVOR_DIAG_H
#define IZVOR_DIAG_H

void diag_set_path(const char *path);

void diag_line_col(const char *src, long offset, int *line, int *col);

void diag_error(const char *src, long offset, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

#endif
