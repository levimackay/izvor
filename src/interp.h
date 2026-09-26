#ifndef IZVOR_INTERP_H
#define IZVOR_INTERP_H

#include "ast.h"

void interp_run(const char *src, const Program *prog);
long interp_eval(const char *src, const Node *expr);

#endif
