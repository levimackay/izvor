/* Abstract syntax tree: node representation and constructors.
   A Node is a tagged union; always check node->type before
   reading anything under node->as. */

#ifndef IZVOR_AST_H
#define IZVOR_AST_H

#include "lexer.h"

typedef enum {
    NODE_NUMBER,
    NODE_UNARY,
    NODE_BINARY
} NodeType;

typedef struct Node Node;

struct Node {
    NodeType type;
    union {
        long number;
        struct {
            TokenType op;
            Node *operand;
        } unary;
        struct {
            TokenType op;
            Node *left;
            Node *right;
        } binary;
    } as;
};

Node *ast_number(long value);
Node *ast_unary(TokenType op, Node *operand);
Node *ast_binary(TokenType op, Node *left, Node *right);
void ast_free(Node *node);

#endif
