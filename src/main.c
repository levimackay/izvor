#include <stdio.h>
#include <stdlib.h>
#include "diag.h"
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
    case NODE_UNARY:
        printf("(- ");
        print_tree(node->as.unary.operand);
        printf(")");
        break;
    case NODE_BINARY: {
        const char *op = "?";
        switch (node->as.binary.op) {
        case TOK_PLUS:  op = "+"; break;
        case TOK_MINUS: op = "-"; break;
        case TOK_STAR:  op = "*"; break;
        case TOK_SLASH: op = "/"; break;
        default: break;
        }
        printf("(%s ", op);
        print_tree(node->as.binary.left);
        printf(" ");
        print_tree(node->as.binary.right);
        printf(")");
        break;
    }
    }
}

static long eval(const Node *node) {
    switch (node->type) {
    case NODE_NUMBER:
        return node->as.number;
    case NODE_UNARY:
        return -eval(node->as.unary.operand);
    case NODE_BINARY: {
        long left = eval(node->as.binary.left);
        long right = eval(node->as.binary.right);
        switch (node->as.binary.op) {
        case TOK_PLUS:  return left + right;
        case TOK_MINUS: return left - right;
        case TOK_STAR:  return left * right;
        case TOK_SLASH:
            if (right == 0) {
                fprintf(stderr, "error: division by zero\n");
                exit(1);
            }
            return left / right;
        default: break;
        }
        break;
    }
    }
    fprintf(stderr, "internal error: unknown node type\n");
    exit(1);
}

static void usage(void) {
    fprintf(stderr,
            "usage: izvor <file.iz>\n"
            "       izvor -e \"<expression>\"\n");
}

int main(int argc, char **argv) {
    char *owned = NULL;
    const char *src = NULL;

    if (argc == 2) {
        owned = read_file(argv[1]);
        if (owned == NULL) {
            return 1;
        }
        src = owned;
        diag_set_path(argv[1]);
    } else if (argc == 3 && argv[1][0] == '-' && argv[1][1] == 'e' && argv[1][2] == '\0') {
        src = argv[2];
        diag_set_path("<command line>");
    } else {
        usage();
        return 1;
    }

    Parser p;
    parser_init(&p, src);
    Node *tree = parser_parse(&p);
    if (tree == NULL) {
        free(owned);
        return 1;
    }

    print_tree(tree);
    printf("\n= %ld\n", eval(tree));

    ast_free(tree);
    free(owned);
    return 0;
}
