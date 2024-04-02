#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdint.h>
#include <stdbool.h>

#include "string.h"
#include "list.h"
#include "lexer.h"

typedef enum expression_type {
    ExpressionType_Empty,
    ExpressionType_Literal,
    ExpressionType_Unary,
    ExpressionType_Binary,
    ExpressionType__count,
} ExpressionType;

typedef enum literal_tag {
    LiteralTag_Number,
    LiteralTag_Symbol,
    LiteralTag__count,
} LiteralTag;

typedef struct expression {
    ExpressionType type;
    bool parenthesised;
} Expression;

typedef struct expression_literal {
    struct expression base;
    LiteralTag tag;
    union {
        double number;
        String symbol;
    };
} Literal;

typedef struct expression_unary {
    struct expression base;
    Token operator;
    Expression* subexpression;
} UnaryExpression;

typedef struct expression_binary {
    struct expression base;
    Token operator;
    Expression* left;
    Expression* right;
} BinaryExpression;

extern Expression* expression_create(const List* const tokens);
extern void expression_destroy(Expression** const expression);
extern void expression_print(const Expression* const expression);

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
