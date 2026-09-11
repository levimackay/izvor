/* Diagnostics: turning a byte offset in the source into a message a
   person can act on. Every stage of the compiler reports through here,
   so an error from the lexer looks the same as one from the parser. */

#ifndef IZVOR_DIAG_H
#define IZVOR_DIAG_H

/* diag_set_path — name the file that errors are reported against.
                   The driver calls this once before compiling.
                   Defaults to "<input>" when nothing sets it. */
void diag_set_path(const char *path);

/* diag_line_col — convert a byte offset into a 1-based line and column.
                   Offsets past the end of the source clamp to the end,
                   because TOK_EOF sits on the terminating NUL. */
void diag_line_col(const char *src, long offset, int *line, int *col);

/* diag_error — print an error naming the file, line and column, then the
                offending line with a caret under the exact character.
                Writes to stderr and returns. Whether to stop is the
                caller's decision, not this function's. */
void diag_error(const char *src, long offset, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

#endif
