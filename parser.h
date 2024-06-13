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
	Expression base;
	LiteralTag tag;
	union {
		double number;
		String symbol;
	};
} Literal;

typedef struct expression_unary {
	Expression base;
	TokenType operator;
	Expression* subexpression;
} UnaryExpression;

typedef struct expression_binary {
	Expression base;
	TokenType operator;
	Expression* left;
	Expression* right;
} BinaryExpression;

// Build expression tree from tokens
extern Expression* expression_parse(const List* const tokens);

// Destroy given expression and its subexpressions recursively
extern void expression_destroy(Expression** const expression);

extern void expression_print(const Expression* const expression);
extern void expression_verbose_print(const Expression* const expression);

// Expression creation functions
extern Literal* expression_literal_create_number(const double number);
extern Literal* expression_literal_create_symbol(String* const symbol);
extern UnaryExpression* expression_unary_create(const TokenType operator,
                                                Expression* const subexpression);
extern BinaryExpression* expression_binary_create(const TokenType operator,
                                                  Expression* const left,
                                                  Expression* const right);

// Check whether expression is of type Empty, used in main function only
extern bool expression_empty(const Expression* const expression);

// Grammar:
//
// input ::= EMPTY | expression
//
// expression ::= literal | unary | binary
//              | '(' expression ')'
//
// unary ::= '+' expression
//         | '-' expression
//
// binary ::= expression '+' expression
//          | expression '-' expression
//          | expression '*' expression
//          | expression '/' expression
//          | expression '^' expression
//
// literal ::= number | symbol
// number ::= "[0-9]+(\.[0-9]*)?"
// symbol ::= [a-zA-Z][a-zA-Z0-9_]*

#endif // __PARSER_H__
