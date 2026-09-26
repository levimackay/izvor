#ifndef IZVOR_PARSER_H
#define IZVOR_PARSER_H

#include <stdbool.h>
#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer lexer;
    Token current;
} Parser;

void parser_init(Parser *p, const char *src);

void parser_advance(Parser *p);

bool parser_check(const Parser *p, TokenType type);

bool parser_match(Parser *p, TokenType type);

Node *parser_parse(Parser *p);

#endif
