#ifndef IZVOR_CHECK_H
#define IZVOR_CHECK_H

#include <stdbool.h>
#include "ast.h"

bool check_program(const char *src, Program *prog);
bool check_expression(const char *src, Node *expr);

#endif
