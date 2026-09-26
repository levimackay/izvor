#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "check.h"
#include "diag.h"
#include "interp.h"
#include "parser.h"

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        fprintf(stderr, "error: cannot open %s\n", path);
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fprintf(stderr, "error: cannot measure %s\n", path);
        fclose(f);
        return NULL;
    }
    long size = ftell(f);
    if (size < 0) {
        fprintf(stderr, "error: cannot measure %s\n", path);
        fclose(f);
        return NULL;
    }
    rewind(f);

    char *buffer = malloc((size_t)size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "error: out of memory reading %s\n", path);
        fclose(f);
        return NULL;
    }

    size_t read = fread(buffer, 1, (size_t)size, f);
    fclose(f);
    if (read != (size_t)size) {
        fprintf(stderr, "error: short read on %s\n", path);
        free(buffer);
        return NULL;
    }

    buffer[read] = '\0';
    return buffer;
}

static void print_tree(const Node *node) {
    switch (node->type) {
    case NODE_NUMBER:
        printf("%ld", node->as.number);
        break;
    case NODE_BOOL:
        printf("%s", node->as.boolean ? "true" : "false");
        break;
    case NODE_NAME:
        printf("%.*s", node->as.name.length, node->as.name.start);
        break;
    case NODE_UNARY:
        printf("(%s ", op_symbol(node->as.unary.op));
        print_tree(node->as.unary.operand);
        printf(")");
        break;
    case NODE_BINARY:
        printf("(%s ", op_symbol(node->as.binary.op));
        print_tree(node->as.binary.left);
        printf(" ");
        print_tree(node->as.binary.right);
        printf(")");
        break;
    case NODE_CALL:
        printf("(%.*s", node->as.call.callee.length, node->as.call.callee.start);
        for (int i = 0; i < node->as.call.arg_count; i++) {
            printf(" ");
            print_tree(node->as.call.args[i]);
        }
        printf(")");
        break;
    }
}

static int evaluate(const char *src) {
    diag_set_path("<command line>");

    Parser p;
    parser_init(&p, src);
    Node *tree = parser_parse(&p);
    if (tree == NULL) {
        return 1;
    }
    if (!check_expression(src, tree)) {
        ast_free(tree);
        return 1;
    }

    print_tree(tree);
    long value = interp_eval(src, tree);
    if (tree->ty == TYPE_BOOL) {
        printf("\n= %s\n", value ? "true" : "false");
    } else {
        printf("\n= %ld\n", value);
    }

    ast_free(tree);
    return 0;
}

static bool front_end(const char *path, char **src, Program *prog) {
    *src = read_file(path);
    if (*src == NULL) {
        return false;
    }
    diag_set_path(path);

    Parser p;
    parser_init(&p, *src);
    *prog = parser_parse_program(&p);
    if (!p.had_error && check_program(*src, prog)) {
        return true;
    }

    program_free(prog);
    free(*src);
    return false;
}

static void usage(void) {
    fprintf(stderr,
            "usage: izvor run <file.iz>\n"
            "       izvor -e \"<expression>\"\n");
}

int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[1], "-e") == 0) {
        return evaluate(argv[2]);
    }

    const char *path = NULL;
    if (argc == 2 && argv[1][0] != '-') {
        path = argv[1];
    } else if (argc == 3 && strcmp(argv[1], "run") == 0) {
        path = argv[2];
    } else {
        usage();
        return 1;
    }

    char *src;
    Program prog;
    if (!front_end(path, &src, &prog)) {
        return 1;
    }

    interp_run(src, &prog);

    program_free(&prog);
    free(src);
    return 0;
}
