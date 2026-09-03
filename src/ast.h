/* Abstract syntax tree: node representation and constructors.
   A Node is a tagged union; always check node->type before
   reading anything under node->as. */

// If not defined, define it
#ifndef IZVOR_AST_H
#define IZVOR_AST_H

// Attaches the lexer.h to this file
#include "lexer.h"

// Here we set up the Node Types
// A node can be 1 of 3 things: a number, a unary, or a binary.
typedef enum {
    NODE_NUMBER,
    NODE_UNARY,
    NODE_BINARY
} NodeType;

// Defined the struct Node
typedef struct Node Node;

// In the struct node, we set the attribute of type, and the two structs for unary and binary.
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

// This promises to the other files that these functions exist
// Doesn't call them or assign them, just tells the other c files that they exist
Node *ast_number(long value);
Node *ast_unary(TokenType op, Node *operand);
Node *ast_binary(TokenType op, Node *left, Node *right);
void ast_free(Node *node);

// Closes the if statment that was opened at the beginning
#endif