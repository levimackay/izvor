#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

void *xrealloc(void *ptr, size_t size) {
    void *grown = realloc(ptr, size);
    if (grown == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    return grown;
}

static Node *new_node(NodeType type, long offset) {
    Node *node = xrealloc(NULL, sizeof(Node));
    node->type = type;
    node->offset = offset;
    node->height = 1;
    node->ty = TYPE_ERROR;
    return node;
}

static int taller(int a, int b) {
    return a > b ? a : b;
}

Node *ast_number(long offset, long value) {
    Node *node = new_node(NODE_NUMBER, offset);
    node->as.number = value;
    return node;
}

Node *ast_bool(long offset, bool value) {
    Node *node = new_node(NODE_BOOL, offset);
    node->as.boolean = value;
    return node;
}

Node *ast_name(long offset, Name name) {
    Node *node = new_node(NODE_NAME, offset);
    node->as.name = name;
    return node;
}

Node *ast_unary(long offset, TokenType op, Node *operand) {
    Node *node = new_node(NODE_UNARY, offset);
    node->as.unary.op = op;
    node->as.unary.operand = operand;
    node->height = operand->height + 1;
    return node;
}

Node *ast_binary(long offset, TokenType op, Node *left, Node *right) {
    Node *node = new_node(NODE_BINARY, offset);
    node->as.binary.op = op;
    node->as.binary.left = left;
    node->as.binary.right = right;
    node->height = taller(left->height, right->height) + 1;
    return node;
}

Node *ast_call(long offset, Name callee, Node **args, int arg_count) {
    Node *node = new_node(NODE_CALL, offset);
    node->as.call.callee = callee;
    node->as.call.args = args;
    node->as.call.arg_count = arg_count;
    for (int i = 0; i < arg_count; i++) {
        node->height = taller(node->height, args[i]->height + 1);
    }
    return node;
}

void ast_free(Node *node) {
    if (node == NULL) {
        return;
    }
    switch (node->type) {
    case NODE_NUMBER:
    case NODE_BOOL:
    case NODE_NAME:
        break;
    case NODE_UNARY:
        ast_free(node->as.unary.operand);
        break;
    case NODE_BINARY:
        ast_free(node->as.binary.left);
        ast_free(node->as.binary.right);
        break;
    case NODE_CALL:
        for (int i = 0; i < node->as.call.arg_count; i++) {
            ast_free(node->as.call.args[i]);
        }
        free(node->as.call.args);
        break;
    }
    free(node);
}

Stmt *stmt_new(StmtType type, long offset) {
    Stmt *stmt = xrealloc(NULL, sizeof(Stmt));
    memset(stmt, 0, sizeof(Stmt));
    stmt->type = type;
    stmt->offset = offset;
    return stmt;
}

void stmt_free(Stmt *stmt) {
    while (stmt != NULL) {
        Stmt *next = stmt->next;
        switch (stmt->type) {
        case STMT_LET:
            ast_free(stmt->as.let.value);
            break;
        case STMT_ASSIGN:
            ast_free(stmt->as.assign.value);
            break;
        case STMT_IF:
            ast_free(stmt->as.if_.cond);
            stmt_free(stmt->as.if_.then_body);
            stmt_free(stmt->as.if_.else_body);
            break;
        case STMT_WHILE:
            ast_free(stmt->as.while_.cond);
            stmt_free(stmt->as.while_.body);
            break;
        case STMT_PRINT:
        case STMT_RETURN:
        case STMT_EXPR:
            ast_free(stmt->as.expr);
            break;
        }
        free(stmt);
        stmt = next;
    }
}

void function_free(Function *fn) {
    while (fn != NULL) {
        Function *next = fn->next;
        free(fn->params);
        stmt_free(fn->body);
        free(fn);
        fn = next;
    }
}

void program_free(Program *prog) {
    function_free(prog->functions);
    stmt_free(prog->statements);
    prog->functions = NULL;
    prog->statements = NULL;
}

bool name_eq(Name a, Name b) {
    return a.length == b.length && memcmp(a.start, b.start, (size_t)a.length) == 0;
}

bool name_is(Name a, const char *text) {
    return (size_t)a.length == strlen(text) && memcmp(a.start, text, (size_t)a.length) == 0;
}

const char *type_name(Type type) {
    switch (type) {
    case TYPE_ERROR: return "<error>";
    case TYPE_INT: return "Int";
    case TYPE_BOOL: return "Bool";
    case TYPE_VOID: return "nothing";
    }
    return "???";
}

const char *op_symbol(TokenType op) {
    switch (op) {
    case TOK_PLUS: return "+";
    case TOK_MINUS: return "-";
    case TOK_STAR: return "*";
    case TOK_SLASH: return "/";
    case TOK_PERCENT: return "%";
    case TOK_BANG: return "!";
    case TOK_EQUAL_EQUAL: return "==";
    case TOK_BANG_EQUAL: return "!=";
    case TOK_LESS: return "<";
    case TOK_LESS_EQUAL: return "<=";
    case TOK_GREATER: return ">";
    case TOK_GREATER_EQUAL: return ">=";
    case TOK_AND: return "&&";
    case TOK_OR: return "||";
    default: return "?";
    }
}
