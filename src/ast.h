#ifndef IZVOR_AST_H
#define IZVOR_AST_H

#include <stdbool.h>
#include <stddef.h>
#include "lexer.h"

typedef enum {
    TYPE_ERROR,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_VOID
} Type;

typedef struct {
    const char *start;
    int length;
} Name;

typedef enum {
    NODE_NUMBER,
    NODE_BOOL,
    NODE_NAME,
    NODE_UNARY,
    NODE_BINARY,
    NODE_CALL
} NodeType;

typedef struct Node Node;

struct Node {
    NodeType type;
    long offset;
    int height;
    Type ty;
    union {
        long number;
        bool boolean;
        Name name;
        struct {
            TokenType op;
            Node *operand;
        } unary;
        struct {
            TokenType op;
            Node *left;
            Node *right;
        } binary;
        struct {
            Name callee;
            Node **args;
            int arg_count;
        } call;
    } as;
};

typedef enum {
    STMT_LET,
    STMT_ASSIGN,
    STMT_PRINT,
    STMT_IF,
    STMT_WHILE,
    STMT_RETURN,
    STMT_EXPR
} StmtType;

typedef struct Stmt Stmt;

struct Stmt {
    StmtType type;
    long offset;
    Stmt *next;
    union {
        struct {
            Name name;
            bool mutable;
            bool annotated;
            Type annotation;
            Node *value;
        } let;
        struct {
            Name name;
            Node *value;
        } assign;
        struct {
            Node *cond;
            Stmt *then_body;
            Stmt *else_body;
        } if_;
        struct {
            Node *cond;
            Stmt *body;
        } while_;
        Node *expr;
    } as;
};

typedef struct {
    Name name;
    Type type;
    long offset;
} Param;

typedef struct Function Function;

struct Function {
    Name name;
    long offset;
    Param *params;
    int param_count;
    Type returns;
    Stmt *body;
    Function *next;
};

typedef struct {
    Function *functions;
    Stmt *statements;
} Program;

void *xrealloc(void *ptr, size_t size);

Node *ast_number(long offset, long value);
Node *ast_bool(long offset, bool value);
Node *ast_name(long offset, Name name);
Node *ast_unary(long offset, TokenType op, Node *operand);
Node *ast_binary(long offset, TokenType op, Node *left, Node *right);
Node *ast_call(long offset, Name callee, Node **args, int arg_count);
void ast_free(Node *node);

Stmt *stmt_new(StmtType type, long offset);
void stmt_free(Stmt *stmt);

void function_free(Function *fn);
void program_free(Program *prog);

bool name_eq(Name a, Name b);
bool name_is(Name a, const char *text);
const char *type_name(Type type);
const char *op_symbol(TokenType op);

#endif
