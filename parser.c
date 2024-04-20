#include "parser.h"
#include "lexer.h"
#include "string.h"
#include "common.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct parser {
	List tokens;
	// @TODO: Put syntax errors here
} Parser;

static const size_t OPERATOR_PRECEDENCE[TokenType__count] = {
	[TokenType_Plus] = 1,
	[TokenType_Minus] = 1,
	[TokenType_Multiply] = 2,
	[TokenType_Divide] = 2,
	[TokenType_Exponent] = 3,
};

static const String OPERATOR_STRING[TokenType__count] = {
	[TokenType_Plus] = String("+"),
	[TokenType_Minus] = String("-"),
	[TokenType_Multiply] = String("*"),
	[TokenType_Divide] = String("/"),
	[TokenType_Exponent] = String("^"),
};

static Expression* parser_parse_input(Parser* const parser);
static Expression* parser_parse_expression(Parser* const parser,
                                           const size_t precedence);
static Expression* parser_parse_literal(Parser* const parser);
static Expression* parser_parse_unary(Parser* const parser);
static Expression* parser_parse_binary(Parser* const parser,
                                       Expression* const lhs,
                                       const size_t precedence);

static Token* parser_next(Parser* const parser);
static Token* parser_peek(Parser* const parser);
static void parser_backup(Parser* const parser);

static void expression_clear(Expression* const expression);
static void expression__print(const Expression* const expression);
static void expression__verbose_print(const Expression* const expression);

static Expression* create_empty_expression(void);

Expression* expression_parse(const List* const tokens)
{
	assert(tokens != NULL);

	Parser parser = {
		.tokens = *tokens,
	};

	return parser_parse_input(&parser);
}

void expression_destroy(Expression** const expression)
{
	assert(expression != NULL);
	expression_clear(*expression);
	*expression = NULL;
}

void expression_print(const Expression* const expression)
{
	assert(expression != NULL);
	expression__print(expression);
	putc('\n', stdout);
}

void expression_verbose_print(const Expression* const expression)
{
	assert(expression != NULL);
	expression__verbose_print(expression);
	putc('\n', stdout);
}

Literal* expression_literal_create_number(const double number)
{
	Literal* const result = malloc(sizeof(Literal));
	if (result == NULL)
		return NULL;

	result->base.type = ExpressionType_Literal;
	result->base.parenthesised = false;
	result->tag = LiteralTag_Number;
	result->number = number;

	return result;
}

Literal* expression_literal_create_symbol(String* const symbol)
{
	assert(symbol != NULL);

	Literal* const result = malloc(sizeof(Literal));
	if (result == NULL)
		return NULL;

	result->base.type = ExpressionType_Literal;
	result->base.parenthesised = false;
	result->tag = LiteralTag_Symbol;
	result->symbol = *symbol;

	return result;
}

UnaryExpression* expression_unary_create(const TokenType operator,
                                         Expression* const subexpression)
{
	assert(token_type_is_unary_operator(operator));
	assert(subexpression != NULL);

	UnaryExpression* const result = malloc(sizeof(UnaryExpression));
	if (result == NULL)
		return NULL;

	result->base.type = ExpressionType_Unary;
	result->base.parenthesised = false;
	result->operator = operator;
	result->subexpression = subexpression;

	return result;
}

BinaryExpression* expression_binary_create(const TokenType operator,
                                           Expression* const left,
                                           Expression* const right)
{
	assert(token_type_is_binary_operator(operator));
	assert(left != NULL);
	assert(right != NULL);

	BinaryExpression* const result = malloc(sizeof(BinaryExpression));
	if (result == NULL)
		return NULL;

	result->base.type = ExpressionType_Binary;
	result->base.parenthesised = false;
	result->operator = operator;
	result->left = left;
	result->right = right;

	return result;
}

bool expression_empty(const Expression* const expression)
{
	assert(expression != NULL);
	return expression->type == ExpressionType_Empty;
}

static Expression* parser_parse_input(Parser* const parser)
{
	assert(parser != NULL);

	Token* const current = parser_peek(parser);

	if (current == NULL)
		return create_empty_expression();

	return parser_parse_expression(parser, 0);
}

static Expression* parser_parse_expression(Parser* const parser,
                                           const size_t precedence)
{
	assert(parser != NULL);

	Token* const current = parser_next(parser);

	Expression* result;

	if (current == NULL)
		return create_empty_expression();
	else if (token_type_is_literal(current->type)) {
		parser_backup(parser);
		result = parser_parse_literal(parser);
	}
	else if (token_type_is_unary_operator(current->type)) {
		parser_backup(parser);
		result = parser_parse_unary(parser);
	}
	else if (current->type == TokenType_LeftParen) {
		result = parser_parse_expression(parser, 0);

		Token* const ahead = parser_next(parser);

		if (ahead == NULL || (ahead != NULL && ahead->type != TokenType_RightParen)) {
			LOG("Syntax error: mismatched \'");
			string_debug_print(&current->content);
			LOGF("\' found at position %lu\n", current->position);

			if (result != NULL)
				expression_clear(result);

			return parser_parse_expression(parser, 0);
		}

		result->parenthesised = true;
	}
	else if (current->type == TokenType_RightParen) {
		LOG("Syntax error: mismatched \'");
		string_debug_print(&current->content);
		LOGF("\' found at position %lu\n", current->position);
		result = parser_parse_expression(parser, 0);
	}
	else {
		LOG("Syntax error: expression expected, found \'");
		string_debug_print(&current->content);
		LOGF("\' at position %lu\n", current->position);
		result = create_empty_expression();
	}

	Token* const ahead = parser_peek(parser);

	if (ahead == NULL)
		return result;

	if (token_type_is_binary_operator(ahead->type)) {
		for (;;) {
			Expression* const binary = parser_parse_binary(parser, result, precedence);

			if (binary == NULL) {
				LOG("Syntax error: expression expected after binary opeartor \'");
				string_debug_print(&ahead->content);
				LOGF("\' at position %lu\n", ahead->position);
				break;
			}

			if (binary == result)
				break;

			parser_backup(parser);
			result = binary;
		}
	}
	else if (ahead->type == TokenType_Symbol) {
		Expression* const rhs = parser_parse_literal(parser);

		BinaryExpression* const binary = expression_binary_create(
			TokenType_Multiply, result, rhs);

		return (Expression*)binary;
	}
	else if (ahead->type == TokenType_LeftParen) {
		Expression* const rhs = parser_parse_expression(parser, 0);

		BinaryExpression* const binary = expression_binary_create(
			TokenType_Multiply, result, (Expression*)rhs);

		return (Expression*)binary;
	}

	return result;
}

static Expression* parser_parse_literal(Parser* const parser)
{
	assert(parser != NULL);

	Token* const literal = parser_next(parser);
	assert(token_type_is_literal(literal->type));

	if (literal->type == TokenType_Number) {
		return (Expression*)expression_literal_create_number(
			string_to_double(&literal->content));
	}
	else if (literal->type == TokenType_Symbol)
		return (Expression*)expression_literal_create_symbol(&literal->content);

	// @NOTE: Never happens
	return NULL;
}

static Expression* parser_parse_unary(Parser* const parser)
{
	assert(parser != NULL);

	Token* const operator = parser_next(parser);
	assert(operator != NULL && token_type_is_unary_operator(operator->type));

	Token* const ahead = parser_peek(parser);

	if (ahead == NULL) {
		LOG("Syntax error: literal or parenthesised expression expected after unary \'");
		string_debug_print(&operator->content);
		LOGF("\' at position %lu\n", operator->position);
		return create_empty_expression();
	}

	if (ahead->type == TokenType_Number) {
		parser_next(parser);

		if (operator->type == TokenType_Minus) {
			return (Expression*)expression_literal_create_number(
				-string_to_double(&ahead->content));
		}

		return (Expression*)expression_literal_create_number(
			string_to_double(&ahead->content));
	}
	else if (ahead->type == TokenType_Symbol) {
		parser_next(parser);

		Expression* const literal = (Expression*)expression_literal_create_symbol(
			&ahead->content);

		if (operator->type == TokenType_Minus)
			return (Expression*)expression_unary_create(operator->type, literal);

		return literal;
	}
	else if (ahead->type == TokenType_LeftParen) {
		return (Expression*)expression_unary_create(operator->type,
			parser_parse_expression(parser, 0));
	}

	LOG("Syntax error: literal or parenthesised expression expected after unary \'");
	string_debug_print(&operator->content);
	LOGF("\' at position %lu\n", operator->position);

	return create_empty_expression();
}

static Expression* parser_parse_binary(Parser* const parser,
                                       Expression* const lhs,
                                       const size_t precedence)
{
	assert(parser != NULL);
	assert(lhs != NULL);

	Token* const operator = parser_next(parser);

	if (operator == NULL || !token_type_is_binary_operator(operator->type))
		return lhs;

	const size_t lhs_precedence = OPERATOR_PRECEDENCE[operator->type];
	const size_t bias = token_type_is_right_associative(operator->type);

	if (lhs_precedence + bias > precedence) {
		Expression* const rhs = parser_parse_expression(parser, lhs_precedence);

		if (rhs == NULL)
			return NULL;

		return (Expression*)expression_binary_create(operator->type, lhs, rhs);
	}

	return lhs;
}

static Token* parser_next(Parser* const parser)
{
	assert(parser);

	if (parser->tokens.head == NULL)
		return NULL;

	ListNode* node = parser->tokens.head;
	parser->tokens.head = parser->tokens.head->next;

	return list_node_data(node, Token);
}

static Token* parser_peek(Parser* const parser)
{
	assert(parser);

	if (parser->tokens.head == NULL)
		return NULL;

	return list_node_data(parser->tokens.head, Token);
}

static void parser_backup(Parser* const parser)
{
	assert(parser);

	if (parser->tokens.head != NULL)
		parser->tokens.head = parser->tokens.head->prev;
	else if (parser->tokens.tail != NULL)
		parser->tokens.head = parser->tokens.tail;
}

static void expression_clear(Expression* const expression)
{
	assert(expression != NULL);

	switch (expression->type) {
	case ExpressionType_Unary: {
		const UnaryExpression* const unary = (UnaryExpression*)expression;
		expression_clear(unary->subexpression);
	} break;

	case ExpressionType_Binary: {
		const BinaryExpression* const binary = (BinaryExpression*)expression;
		expression_clear(binary->left);
		expression_clear(binary->right);
	} break;
	}

	free(expression);
}

static void expression__print(const Expression* const expression)
{
	assert(expression != NULL);

	switch (expression->type) {
	case ExpressionType_Empty: {
		fputs("()", stdout);
	} break;

	case ExpressionType_Literal: {
		const Literal* const literal = (Literal*)expression;

		const bool parenthesised = literal->base.parenthesised;

		if (parenthesised)
			putc('(', stdout);

		switch (literal->tag) {
		case LiteralTag_Number:
			fprintf(stdout, "%.12g", literal->number);
			break;

		case LiteralTag_Symbol:
			string_print(&literal->symbol);
			break;
		}

		if (parenthesised)
			putc(')', stdout);
	} break;

	case ExpressionType_Unary: {
		const UnaryExpression* const unary = (UnaryExpression*)expression;
		string_print(&OPERATOR_STRING[unary->operator]);
		expression__print(unary->subexpression);
	} break;

	case ExpressionType_Binary: {
		const BinaryExpression* const binary = (BinaryExpression*)expression;

		const bool parenthesised = binary->base.parenthesised;

		if (parenthesised)
			putc('(', stdout);

		expression__print(binary->left);
		putc(' ', stdout);

		string_print(&OPERATOR_STRING[binary->operator]);
		putc(' ', stdout);

		expression__print(binary->right);

		if (parenthesised)
			putc(')', stdout);
	} break;
	}
}

static void expression__verbose_print(const Expression* const expression)
{
	assert(expression != NULL);

	switch (expression->type) {
	case ExpressionType_Literal: {
		const Literal* const literal = (Literal*)expression;

		switch (literal->tag) {
		case LiteralTag_Number:
			fprintf(stdout, "%.12g", literal->number);
			break;

		case LiteralTag_Symbol:
			string_print(&literal->symbol);
			break;
		}
	} break;

	case ExpressionType_Unary: {
		const UnaryExpression* const unary = (UnaryExpression*)expression;
		putc('{', stdout);
		string_print(&OPERATOR_STRING[unary->operator]);
		putc(' ', stdout);
		expression__verbose_print(unary->subexpression);
		putc('}', stdout);
	} break;

	case ExpressionType_Binary: {
		const BinaryExpression* const binary = (BinaryExpression*)expression;

		putc('{', stdout);
		string_print(&OPERATOR_STRING[binary->operator]);
		putc(' ', stdout);

		expression__verbose_print(binary->left);
		putc(' ', stdout);

		expression__verbose_print(binary->right);
		putc('}', stdout);
	} break;
	}
}

static Expression* create_empty_expression(void)
{
	Expression* const result = malloc(sizeof(Expression));
	if (result == NULL)
		return NULL;

	result->type = ExpressionType_Empty;
	result->parenthesised = false;

	return result;
}

