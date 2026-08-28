/* Phase 1 — the lexer's public interface.
 * Read every comment here before writing any code (task 1.1).
 * Doc: docs/phases/01-lexer.md
 */
#ifndef TINYLANG_LEXER_H
#define TINYLANG_LEXER_H

/* Every kind of "word" tinylang currently understands.
 * Later phases append to this enum (keywords, identifiers, comparisons). */
typedef enum {
    TOK_NUMBER,   /* 123        */
    TOK_PLUS,     /* +          */
    TOK_MINUS,    /* -          */
    TOK_STAR,     /* *          */
    TOK_SLASH,    /* /          */
    TOK_LPAREN,   /* (          */
    TOK_RPAREN,   /* )          */
    TOK_EOF,      /* end of the source string */
    TOK_ERROR     /* a character we don't recognize */
} TokenType;

/* One token. Note what it does NOT contain: a copied string.
 * `start` points INTO the original source text and `length` says how many
 * characters belong to this token. That means zero allocations per token —
 * but it also means the source string must outlive every token cut from
 * it. (For TOK_ERROR, start/length cover the offending character.) */
typedef struct {
    TokenType type;
    const char *start;   /* first character of the lexeme, in the source */
    int length;          /* number of characters in the lexeme */
    long value;          /* the parsed number — meaningful only for TOK_NUMBER */
} Token;

/* The lexer's entire state: where the text is, and how far we've read.
 * That's it. A lexer is small. */
typedef struct {
    const char *src;   /* the whole source string (NUL-terminated) */
    int pos;           /* index of the next character to read */
} Lexer;

/* Task 1.1: printable name for a token type ("NUMBER", "PLUS", ...).
 * Purely for debugging and lexdump — but you'll use it constantly. */
const char *token_type_name(TokenType type);

/* Task 1.2: point the lexer at a source string, ready to read from the top. */
void lexer_init(Lexer *lx, const char *src);

/* Tasks 1.2–1.4: produce the next token. The contract:
 *   - skips any whitespace first (task 1.3)
 *   - at the end of the source, returns TOK_EOF — and keeps returning
 *     TOK_EOF forever if called again (callers rely on this)
 *   - on an unrecognized character, returns TOK_ERROR covering that one
 *     character and moves past it (task 1.4)
 */
Token lexer_next(Lexer *lx);

#endif
