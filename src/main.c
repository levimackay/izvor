/* izvor driver: read a program, parse it, print the syntax tree in
   S-expression form, then evaluate it.

   Input comes from a file (izvor prog.iz) or straight from the command
   line (izvor -e "1 + 2"). Both paths end up as one NUL-terminated
   buffer that the whole front end borrows and never owns. */

#include <stdio.h>
#include <stdlib.h>
#include "diag.h"
#include "parser.h"

/* read_file — slurp a whole file into one NUL-terminated buffer.
   Reading the file in one shot rather than streaming it is what lets a
   Token be a pointer into the source instead of a copied string, which
   is why error messages can print the offending line back. Caller owns
   the buffer. Returns NULL after reporting the reason. */
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

    // One extra byte for the NUL the lexer stops on.
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

/* print_tree — write the tree back out as an S-expression. Parentheses
   make the parser's precedence decisions visible, so 1 + 2 * 3 prints as
   (+ 1 (* 2 3)) and a wrong answer is obvious by eye. */
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

/* eval — the meaning of a tree, defined by recursion: a number means
   itself, an operator node means its children combined. This is the
   temporary back end. Once codegen lands it stays as the oracle the
   generated C has to agree with. */
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
            // C leaves integer division by zero undefined, which means
            // the compiler may do literally anything. izvor decides
            // instead: it is an error, and the program stops.
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
