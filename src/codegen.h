#ifndef IZVOR_CODEGEN_H
#define IZVOR_CODEGEN_H

#include <stdio.h>
#include "ast.h"

void codegen_emit(FILE *out, const char *src, const Program *prog);

#endif
