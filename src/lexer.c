/* Phase 1 — lexer implementation. All TODOs are yours.
 * Doc: docs/phases/01-lexer.md
 */
#include "lexer.h"

/* ------------------------------------------------------------------ */
/* Suggested private helpers (task 1.2). `static` means "visible only  */
/* inside this file" — the C version of private.                       */
/*                                                                     */
/*   static char peek(const Lexer *lx);    what is the next char?      */
/*                                         ('\0' at end of source)     */
/*   static char advance(Lexer *lx);       consume + return next char  */
/*                                                                     */
/* Everything below is built from these two. In task 1.3 you'll likely */
/* add:                                                                */
/*                                                                     */
/*   static int is_digit(char c);                                      */
/*   static void skip_whitespace(Lexer *lx);   spaces, tabs, newlines  */
/*                                                                     */
/* And a constructor helper saves a lot of typing:                     */
/*                                                                     */
/*   static Token make_token(const Lexer *lx, TokenType type,          */
/*                           int start_pos);                           */
/* ------------------------------------------------------------------ */

/* Task 1.1 — a switch over every TokenType.
 * Tip: leave out the `default:` case. With -Wall, clang then WARNS you
 * whenever a future TokenType is missing from the switch — the compiler
 * becomes your checklist. */
const char *token_type_name(TokenType type) {
    switch (type){
    case TOK_NUMBER:
        return "NUMBER";
    case TOK_PLUS:
        return "PLUS";
    case TOK_MINUS:
        return "MINUS";
    case TOK_STAR:
        return "STAR";
    case TOK_SLASH:
        return "SLASH";
    case TOK_LPAREN:
        return "LPAREN";
    case TOK_RPAREN:
        return "RPAREN";
    case TOK_EOF:
        return "EOF";
    case TOK_ERROR:
        return "ERROR";
}
}

void lexer_init(Lexer *lx, const char *src) {
    lx->src = src;
    lx->pos = 0;
}

static char peek(const Lexer *lx) {
    return lx->src[lx->pos];
}

static char advance(Lexer *lx) {
    return lx->src[lx->pos++];
}

static Token make_token(const Lexer *lx, TokenType type, int start_pos) {
    Token t;
    t.type = type;
    t.start = lx->src + start_pos;
    t.length = lx->pos - start_pos;
    t.value = 0;
    return t;
}

/* Tasks 1.2–1.4 — the heart of the lexer.
 * Shape:
 *   1. skip whitespace                       (task 1.3)
 *   2. remember where this token starts
 *   3. if at '\0': return an EOF token WITHOUT advancing past the
 *      terminator (that's what makes repeated calls keep saying EOF)
 *   4. if it's a digit: consume the whole run of digits, computing the
 *      value as you go (your parse_uint from task 0.6)   (task 1.3)
 *   5. otherwise switch on the character: + - * / ( )    (task 1.2)
 *   6. anything else: TOK_ERROR covering that character  (task 1.4)
 */
Token lexer_next(Lexer *lx) {
    (void)lx;
    Token t = {TOK_ERROR, 0, 0, 0};
    return t; /* TODO */
}
