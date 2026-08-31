/* Parser public interface: token helpers and expression parsing. */

#ifndef IZVOR_PARSER_H
#define IZVOR_PARSER_H

#include <stdbool.h>
#include "lexer.h"
#include "ast.h"

/* Parser: a lexer + one token of lookahead.
   The lexer has no rewind, so the parser holds the token
   it has fetched but not yet consumed. */
typedef struct {
    Lexer lexer;
    Token current;
} Parser;

/* parser_init  — set up the lexer, then advance once so
                  current is loaded before anyone reads it */
void parser_init(Parser *p, const char *src);

/* parser_advance — pull the next token into current */
void parser_advance(Parser *p);

/* parser_check — is current this type? asks only, consumes nothing */
bool parser_check(const Parser *p, TokenType type);

/* parser_match — if current is this type, consume it and say true;
                  otherwise change nothing and say false */
bool parser_match(Parser *p, TokenType type);

/* parser_parse — parse the whole input as one expression, requiring EOF
                  at the end. Returns the tree, or NULL after printing an
                  error to stderr. Caller owns the tree (ast_free). */
Node *parser_parse(Parser *p);

#endif
