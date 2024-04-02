#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdint.h>
#include <stdbool.h>

#include "string.h"
#include "list.h"

typedef enum ast_type {
    AstType_empty,
    AstType_literal,
    AstType_unary_operator,
    AstType_binary_operator,
    AstType__count,
} AstType;

typedef enum ast_literal_type {
    AstLiteralType_number,
    AstLiteralType_symbol,
    AstLitearlType__count,
} AstLiteralType;

typedef enum ast_operator {
    AstOperator_add,
    AstOperator_subtract,
    AstOperator_multiply,
    AstOperator_divide,
    AstOperator_raise_to,
    AstOperator_unary_plus,
    AstOperator_unary_minus,
    AstOperator__count,
} AstOperator;

typedef struct ast_expression {
    bool parenthesised;
} AstExpression;

typedef struct ast_literal {
    AstExpression base;
    const AstLiteralType type;
    union {
        double number;
        String symbol;
    };
} AstLiteral;

typedef struct ast_unary_operator {
    AstExpression base;
    AstOperator operator;
    AstExpression* subexpression;
} AstUnaryOperator;

typedef struct ast_binary_operator {
    AstExpression base;
    AstOperator operator;
    AstExpression* left;
    AstExpression* right;
} AstBinaryOperator;

typedef struct ast {
    AstType type;
    union {
        struct ast_literal literal;
        struct ast_unary_operator unary;
        struct ast_binary_operator binary;
    };
} Ast;

extern Ast* ast_parse(const List* const tokens);
extern void ast_destroy(Ast** const ast);
extern void ast_print(const Ast* const ast);

// Grammar:
//
// input ::= EMPTY | '(' input ')' | expression
//
// expression ::= literal | unary | binary
//              | '(' expression ')'
//
// unary ::= '+' {literal | binary}
//         | '-' {literal | binary}
//
// binary ::= expression '+' expression
//          | expression '-' expression
//          | expression '*' expression
//          | expression '/' expression
//          | expression '^' expression

#endif // __PARSER_H__
