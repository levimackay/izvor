/* Include guard: IZVOR_PARSER_H */
/* Needs lexer.h (Lexer, Token) and ast.h (Node) */

/* Parser: a lexer + one token of lookahead.
   The lexer has no rewind, so the parser holds the token
   it has fetched but not yet consumed. */

/* Declarations: parser_init, parser_advance, parser_check, parser_match */