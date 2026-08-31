/* AST implementation: heap-allocated node constructors and the
   recursive tree destructor. */

// Pretty much the same thing as attaching your CSS to HTML
// stdlib is the standard library for C (like "using System" in C#)
#include <stdlib.h>
// This brings in our other file that defines Node and Nodetype so we can use them here
#include "ast.h"


// Private function that creates a new node and returns the pointer (address) to the new node
// Input is the NodeType as defined in the ast.h
static Node *new_node(NodeType type) {
    // Heap allocation. The compiler knows how much space one node needs
    // Returns the pointer (address) in the variable node.
    Node *node = malloc(sizeof(Node));
    // Failure check
    // If the system doesn't have the memory to give you it will return null
    if (node == NULL) {
        return NULL;
    }
    // follow the pointer, stamp the kind tag into the struct's type field
    node->type = type;
    return node;
}

// Fn takes the value of the number and stores it as long.
Node *ast_number(long value) {
    // ask new_node for an empty node already tagged NODE_NUMBER
    Node *node = new_node(NODE_NUMBER);
    // Same failure check as before
    if (node == NULL) {
        return NULL;
    }
    //follow the pointer, stores the payload
    node->as.number = value;
    return node;
}

// New function for operators in the tree. Takes in the Token type and the pointer to the operand
Node *ast_unary(TokenType op, Node *operand) {
    // Creates a new node for the operator
    Node *node = new_node(NODE_UNARY);
    // Failure check
    if (node == NULL) {
        return NULL;
    }
    // Stores the which operator it is 
    node->as.unary.op = op;
    // Stores the operand
    node->as.unary.operand = operand;
    return node;
}

// New function that takes in the operator, and what nodes are to the left and right of it
Node *ast_binary(TokenType op, Node *left, Node *right) {
    // Creates a new node
    Node *node = new_node(NODE_BINARY);
    if (node == NULL) {
        return NULL;
    }
    // Stores the operator
    node->as.binary.op = op;
    // Stores the node to the left
    node->as.binary.left = left;
    // stores the node to the right
    node->as.binary.right = right;
    return node;
}

// This function at the end goes and clears up the memory that we allocated for all of these things
void ast_free(Node *node) {
    // BASE CASE
    // Can't free nothing; base case that stops the recursion
    if (node == NULL) {
        return;
    }
    // check the tag before touching the union, so each case frees only the children that node kind actually has
    // uses recursion to free up all the nodes
    // frees the children first and then the parents
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
