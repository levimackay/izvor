#include <stdlib.h>
#include "ast.h"

static Node *new_node(NodeType type) {
    Node *node = malloc(sizeof(Node));
    if (node == NULL) {
        return NULL;
    }
    node->type = type;
    return node;
}

Node *ast_number(long value) {
    Node *node = new_node(NODE_NUMBER);
    if (node == NULL) {
        return NULL;
    }
    node->as.number = value;
    return node;
}

Node *ast_unary(TokenType op, Node *operand) {
    Node *node = new_node(NODE_UNARY);
    if (node == NULL) {
        return NULL;
    }
    node->as.unary.op = op;
    node->as.unary.operand = operand;
    return node;
}

Node *ast_binary(TokenType op, Node *left, Node *right) {
    Node *node = new_node(NODE_BINARY);
    if (node == NULL) {
        return NULL;
    }
    node->as.binary.op = op;
    node->as.binary.left = left;
    node->as.binary.right = right;
    return node;
}

void ast_free(Node *node) {
    if (node == NULL) {
        return;
    }
    switch (node->type) {
    case NODE_NUMBER:
        break;
    case NODE_UNARY:
        ast_free(node->as.unary.operand);
        break;
    case NODE_BINARY:
        ast_free(node->as.binary.left);
        ast_free(node->as.binary.right);
        break;
    }
    free(node);
}
