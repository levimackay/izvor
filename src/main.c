/* izvor CLI: parse one expression from the command line, print its
   syntax tree in S-expression form, then evaluate it. */

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

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
                fprintf(stderr, "runtime error: division by zero\n");
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

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: izvor \"<expression>\"\n");
        return 1;
    }

    Parser p;
    parser_init(&p, argv[1]);
    Node *tree = parser_parse(&p);
    if (tree == NULL) return 1;

    print_tree(tree);
    printf("\n= %ld\n", eval(tree));

    ast_free(tree);
    return 0;
}