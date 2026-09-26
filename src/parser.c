#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "diag.h"
#include "parser.h"

void parser_init(Parser *p, const char *src) {
    lexer_init(&p->lexer, src);
    p->had_error = false;
    p->depth = 0;
    p->block_depth = 0;
    p->current.type = TOK_EOF;
    p->current.start = src;
    p->current.length = 0;
    p->current.value = 0;
    parser_advance(p);
}

void parser_advance(Parser *p) {
    p->previous = p->current;
    p->current = lexer_next(&p->lexer);
}

bool parser_check(const Parser *p, TokenType type) {
    return p->current.type == type;
}

bool parser_match(Parser *p, TokenType type) {
    if (!parser_check(p, type)) return false;
    parser_advance(p);
    return true;
}

static long offset_of(const Parser *p, Token t) {
    return t.start - p->lexer.src;
}

static Name name_of(Token t) {
    Name name = {t.start, t.length};
    return name;
}

static void error_at(Parser *p, const char *msg) {
    Token t = p->current;
    const char *src = p->lexer.src;
    long offset = offset_of(p, t);
    unsigned char c = (unsigned char)t.start[0];

    p->had_error = true;
    if (t.type == TOK_ERROR && isdigit(c)) {
        diag_error(src, offset, "integer literal is too large");
    } else if (t.type == TOK_ERROR && c >= 0x80) {
        diag_error(src, offset, "non-ASCII characters are only allowed in comments");
    } else if (t.type == TOK_ERROR && isprint(c)) {
        diag_error(src, offset, "unexpected character '%c'", c);
    } else if (t.type == TOK_ERROR) {
        diag_error(src, offset, "unexpected control character 0x%02x", c);
    } else if (t.type == TOK_EOF) {
        diag_error(src, offset, "%s, found end of file", msg);
    } else {
        diag_error(src, offset, "%s, found '%.*s'", msg, t.length, t.start);
    }
}

static void error_here(Parser *p, long offset, const char *msg) {
    p->had_error = true;
    diag_error(p->lexer.src, offset, "%s", msg);
}

static bool expect(Parser *p, TokenType type, const char *msg) {
    if (parser_match(p, type)) return true;
    if (type == TOK_SEMICOLON && p->current.type != TOK_ERROR) {
        Token last = p->previous;
        error_here(p, offset_of(p, last) + last.length, "expected ';'");
        return false;
    }
    error_at(p, msg);
    return false;
}

static bool enter(Parser *p, const char *msg) {
    if (p->depth >= MAX_NESTING) {
        error_here(p, offset_of(p, p->current), msg);
        return false;
    }
    p->depth++;
    return true;
}

static Node *limit(Parser *p, Node *node) {
    if (node->height <= MAX_NESTING) return node;
    error_here(p, node->offset, "expression is nested too deeply");
    ast_free(node);
    return NULL;
}

static bool parse_type(Parser *p, Type *out) {
    if (!parser_check(p, TOK_IDENT)) {
        error_at(p, "expected a type");
        return false;
    }
    Name name = name_of(p->current);
    if (name_is(name, "Int")) {
        *out = TYPE_INT;
    } else if (name_is(name, "Bool")) {
        *out = TYPE_BOOL;
    } else {
        p->had_error = true;
        diag_error(p->lexer.src, offset_of(p, p->current),
                   "unknown type '%.*s'", name.length, name.start);
        return false;
    }
    parser_advance(p);
    return true;
}

static Node *parse_expression(Parser *p);

static Node *parse_call(Parser *p, Token callee) {
    Node **args = NULL;
    int count = 0;

    if (!parser_check(p, TOK_RPAREN)) {
        do {
            Node *arg = parse_expression(p);
            if (arg == NULL) goto fail;
            args = xrealloc(args, sizeof(Node *) * (size_t)(count + 1));
            args[count++] = arg;
        } while (parser_match(p, TOK_COMMA));
    }
    if (!expect(p, TOK_RPAREN, "expected ')'")) goto fail;
    return limit(p, ast_call(offset_of(p, callee), name_of(callee), args, count));

fail:
    for (int i = 0; i < count; i++) {
        ast_free(args[i]);
    }
    free(args);
    return NULL;
}

static Node *parse_atom(Parser *p) {
    Token t = p->current;
    long offset = offset_of(p, t);

    switch (t.type) {
    case TOK_NUMBER:
        parser_advance(p);
        return ast_number(offset, t.value);
    case TOK_TRUE:
    case TOK_FALSE:
        parser_advance(p);
        return ast_bool(offset, t.type == TOK_TRUE);
    case TOK_IDENT:
        parser_advance(p);
        if (parser_match(p, TOK_LPAREN)) return parse_call(p, t);
        return ast_name(offset, name_of(t));
    case TOK_LPAREN: {
        parser_advance(p);
        Node *inner = parse_expression(p);
        if (inner == NULL) return NULL;
        if (!expect(p, TOK_RPAREN, "expected ')'")) {
            ast_free(inner);
            return NULL;
        }
        return inner;
    }
    default:
        error_at(p, "expected expression");
        return NULL;
    }
}

static Node *parse_factor(Parser *p) {
    if (!enter(p, "expression is nested too deeply")) return NULL;

    Node *node;
    if (parser_check(p, TOK_MINUS) || parser_check(p, TOK_BANG)) {
        Token op = p->current;
        parser_advance(p);
        Node *operand = parse_factor(p);
        node = operand == NULL ? NULL : limit(p, ast_unary(offset_of(p, op), op.type, operand));
    } else {
        node = parse_atom(p);
    }

    p->depth--;
    return node;
}

static bool at_any(const Parser *p, const TokenType *ops) {
    for (; *ops != TOK_EOF; ops++) {
        if (parser_check(p, *ops)) return true;
    }
    return false;
}

static Node *parse_left_assoc(Parser *p, Node *(*operand)(Parser *), const TokenType *ops) {
    Node *left = operand(p);
    if (left == NULL) return NULL;
    while (at_any(p, ops)) {
        Token op = p->current;
        parser_advance(p);
        Node *right = operand(p);
        if (right == NULL) {
            ast_free(left);
            return NULL;
        }
        left = limit(p, ast_binary(offset_of(p, op), op.type, left, right));
        if (left == NULL) return NULL;
    }
    return left;
}

static Node *parse_term(Parser *p) {
    static const TokenType ops[] = {TOK_STAR, TOK_SLASH, TOK_PERCENT, TOK_EOF};
    return parse_left_assoc(p, parse_factor, ops);
}

static Node *parse_sum(Parser *p) {
    static const TokenType ops[] = {TOK_PLUS, TOK_MINUS, TOK_EOF};
    return parse_left_assoc(p, parse_term, ops);
}

static Node *parse_comparison(Parser *p) {
    static const TokenType ops[] = {
        TOK_LESS, TOK_LESS_EQUAL, TOK_GREATER, TOK_GREATER_EQUAL, TOK_EOF
    };
    return parse_left_assoc(p, parse_sum, ops);
}

static Node *parse_equality(Parser *p) {
    static const TokenType ops[] = {TOK_EQUAL_EQUAL, TOK_BANG_EQUAL, TOK_EOF};
    return parse_left_assoc(p, parse_comparison, ops);
}

static Node *parse_and(Parser *p) {
    static const TokenType ops[] = {TOK_AND, TOK_EOF};
    return parse_left_assoc(p, parse_equality, ops);
}

static Node *parse_expression(Parser *p) {
    static const TokenType ops[] = {TOK_OR, TOK_EOF};
    return parse_left_assoc(p, parse_and, ops);
}

Node *parser_parse(Parser *p) {
    Node *expr = parse_expression(p);
    if (expr == NULL) return NULL;
    if (!parser_check(p, TOK_EOF)) {
        error_at(p, "expected end of input");
        ast_free(expr);
        return NULL;
    }
    return expr;
}

static void synchronize(Parser *p, bool moved) {
    int nesting = 0;

    for (;;) {
        switch (p->current.type) {
        case TOK_EOF:
            return;
        case TOK_LBRACE:
            nesting++;
            break;
        case TOK_RBRACE:
            if (nesting == 0 && p->block_depth > 0) return;
            if (nesting > 0) nesting--;
            break;
        case TOK_SEMICOLON:
            if (nesting == 0) {
                parser_advance(p);
                return;
            }
            break;
        case TOK_LET:
        case TOK_VAR:
        case TOK_PRINT:
        case TOK_IF:
        case TOK_WHILE:
        case TOK_RETURN:
        case TOK_FN:
            if (nesting == 0 && moved) return;
            break;
        default:
            break;
        }
        parser_advance(p);
        moved = true;
    }
}

static Stmt *parse_statement(Parser *p);

static bool parse_block(Parser *p, Stmt **body) {
    *body = NULL;
    if (!enter(p, "blocks are nested too deeply")) return false;
    if (!expect(p, TOK_LBRACE, "expected '{'")) {
        p->depth--;
        return false;
    }

    p->block_depth++;
    Stmt **tail = body;
    while (!parser_check(p, TOK_RBRACE) && !parser_check(p, TOK_EOF)) {
        const char *before = p->current.start;
        Stmt *stmt = parse_statement(p);
        if (stmt == NULL) {
            synchronize(p, p->current.start != before);
            continue;
        }
        *tail = stmt;
        tail = &stmt->next;
    }
    p->block_depth--;
    p->depth--;

    if (!expect(p, TOK_RBRACE, "expected '}'")) {
        stmt_free(*body);
        *body = NULL;
        return false;
    }
    return true;
}

static Stmt *parse_let(Parser *p) {
    bool mutable = parser_check(p, TOK_VAR);
    long offset = offset_of(p, p->current);
    parser_advance(p);

    if (!parser_check(p, TOK_IDENT)) {
        error_at(p, "expected a name");
        return NULL;
    }
    Stmt *stmt = stmt_new(STMT_LET, offset);
    stmt->as.let.name = name_of(p->current);
    stmt->as.let.mutable = mutable;
    parser_advance(p);

    if (parser_match(p, TOK_COLON)) {
        if (!parse_type(p, &stmt->as.let.annotation)) goto fail;
        stmt->as.let.annotated = true;
    }
    if (!expect(p, TOK_EQUAL, "expected '='")) goto fail;
    stmt->as.let.value = parse_expression(p);
    if (stmt->as.let.value == NULL) goto fail;
    if (!expect(p, TOK_SEMICOLON, "expected ';'")) goto fail;
    return stmt;

fail:
    stmt_free(stmt);
    return NULL;
}

static Stmt *parse_print(Parser *p) {
    Stmt *stmt = stmt_new(STMT_PRINT, offset_of(p, p->current));
    parser_advance(p);

    if (!expect(p, TOK_LPAREN, "expected '('")) goto fail;
    stmt->as.expr = parse_expression(p);
    if (stmt->as.expr == NULL) goto fail;
    if (!expect(p, TOK_RPAREN, "expected ')'")) goto fail;
    if (!expect(p, TOK_SEMICOLON, "expected ';'")) goto fail;
    return stmt;

fail:
    stmt_free(stmt);
    return NULL;
}

static Stmt *parse_if(Parser *p) {
    if (!enter(p, "blocks are nested too deeply")) return NULL;
    Stmt *stmt = stmt_new(STMT_IF, offset_of(p, p->current));
    parser_advance(p);

    stmt->as.if_.cond = parse_expression(p);
    if (stmt->as.if_.cond == NULL) goto fail;
    if (!parse_block(p, &stmt->as.if_.then_body)) goto fail;

    if (parser_match(p, TOK_ELSE)) {
        if (parser_check(p, TOK_IF)) {
            stmt->as.if_.else_body = parse_if(p);
            if (stmt->as.if_.else_body == NULL) goto fail;
        } else if (!parse_block(p, &stmt->as.if_.else_body)) {
            goto fail;
        }
    }
    p->depth--;
    return stmt;

fail:
    p->depth--;
    stmt_free(stmt);
    return NULL;
}

static Stmt *parse_while(Parser *p) {
    Stmt *stmt = stmt_new(STMT_WHILE, offset_of(p, p->current));
    parser_advance(p);

    stmt->as.while_.cond = parse_expression(p);
    if (stmt->as.while_.cond == NULL) goto fail;
    if (!parse_block(p, &stmt->as.while_.body)) goto fail;
    return stmt;

fail:
    stmt_free(stmt);
    return NULL;
}

static Stmt *parse_return(Parser *p) {
    Stmt *stmt = stmt_new(STMT_RETURN, offset_of(p, p->current));
    parser_advance(p);

    if (!parser_check(p, TOK_SEMICOLON)) {
        stmt->as.expr = parse_expression(p);
        if (stmt->as.expr == NULL) goto fail;
    }
    if (!expect(p, TOK_SEMICOLON, "expected ';'")) goto fail;
    return stmt;

fail:
    stmt_free(stmt);
    return NULL;
}

static Stmt *parse_simple(Parser *p) {
    long offset = offset_of(p, p->current);
    Node *expr = parse_expression(p);
    if (expr == NULL) return NULL;

    Stmt *stmt;
    if (parser_check(p, TOK_EQUAL)) {
        if (expr->type != NODE_NAME) {
            error_here(p, offset, "only a variable can be assigned to");
            ast_free(expr);
            return NULL;
        }
        parser_advance(p);
        stmt = stmt_new(STMT_ASSIGN, offset);
        stmt->as.assign.name = expr->as.name;
        ast_free(expr);
        stmt->as.assign.value = parse_expression(p);
        if (stmt->as.assign.value == NULL) goto fail;
    } else {
        stmt = stmt_new(STMT_EXPR, offset);
        stmt->as.expr = expr;
    }
    if (!expect(p, TOK_SEMICOLON, "expected ';'")) goto fail;
    return stmt;

fail:
    stmt_free(stmt);
    return NULL;
}

static Stmt *parse_statement(Parser *p) {
    switch (p->current.type) {
    case TOK_LET:
    case TOK_VAR:
        return parse_let(p);
    case TOK_PRINT:
        return parse_print(p);
    case TOK_IF:
        return parse_if(p);
    case TOK_WHILE:
        return parse_while(p);
    case TOK_RETURN:
        return parse_return(p);
    case TOK_FN:
        error_here(p, offset_of(p, p->current),
                   "functions can only be declared at the top level");
        return NULL;
    default:
        return parse_simple(p);
    }
}

static Function *parse_function(Parser *p) {
    Function *fn = xrealloc(NULL, sizeof(Function));
    memset(fn, 0, sizeof(Function));
    fn->returns = TYPE_VOID;
    parser_advance(p);

    if (!parser_check(p, TOK_IDENT)) {
        error_at(p, "expected a function name");
        goto fail;
    }
    fn->name = name_of(p->current);
    fn->offset = offset_of(p, p->current);
    parser_advance(p);

    if (!expect(p, TOK_LPAREN, "expected '('")) goto fail;
    if (!parser_check(p, TOK_RPAREN)) {
        do {
            if (!parser_check(p, TOK_IDENT)) {
                error_at(p, "expected a parameter name");
                goto fail;
            }
            Param param;
            param.name = name_of(p->current);
            param.offset = offset_of(p, p->current);
            parser_advance(p);
            if (!expect(p, TOK_COLON, "expected ':'")) goto fail;
            if (!parse_type(p, &param.type)) goto fail;

            fn->params = xrealloc(fn->params, sizeof(Param) * (size_t)(fn->param_count + 1));
            fn->params[fn->param_count++] = param;
        } while (parser_match(p, TOK_COMMA));
    }
    if (!expect(p, TOK_RPAREN, "expected ')'")) goto fail;
    if (parser_match(p, TOK_ARROW) && !parse_type(p, &fn->returns)) goto fail;
    if (!parse_block(p, &fn->body)) goto fail;
    return fn;

fail:
    function_free(fn);
    return NULL;
}

Program parser_parse_program(Parser *p) {
    Program prog = {NULL, NULL};
    Function **fn_tail = &prog.functions;
    Stmt **stmt_tail = &prog.statements;

    while (!parser_check(p, TOK_EOF)) {
        const char *before = p->current.start;
        if (parser_check(p, TOK_FN)) {
            Function *fn = parse_function(p);
            if (fn == NULL) {
                synchronize(p, p->current.start != before);
                continue;
            }
            *fn_tail = fn;
            fn_tail = &fn->next;
        } else {
            Stmt *stmt = parse_statement(p);
            if (stmt == NULL) {
                synchronize(p, p->current.start != before);
                continue;
            }
            *stmt_tail = stmt;
            stmt_tail = &stmt->next;
        }
    }
    return prog;
}
