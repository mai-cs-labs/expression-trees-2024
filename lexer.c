#include "lexer.h"

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

typedef struct lexer {
	const String* const input;
	size_t start;
	size_t position;
	List tokens;
	bool stop;
} Lexer;

typedef void* (*LexerState)(Lexer* const lexer);
typedef LexerState (*LexerStateFn)(Lexer* const lexer);

static LexerState lexer_scan_text(Lexer* const lexer);
static LexerState lexer_scan_number(Lexer* const lexer);
static LexerState lexer_scan_symbol(Lexer* const lexer);

static uint8_t lexer_next(Lexer* const lexer);
static void lexer_backup(Lexer* const lexer);
static uint8_t lexer_peek(Lexer* const lexer);
static void lexer_ignore(Lexer* const lexer);
static void lexer_emit(Lexer* const lexer, const TokenType type);

static bool is_operator(const uint8_t c);

List lexical_scan(const String* const string)
{
	assert(string != NULL);

	Lexer lexer = {
		.input = string,
		.position = 0,
		.start = 0,
		.tokens = List(),
		.stop = false,
	};

	LexerStateFn state = lexer_scan_text;
	while (state != NULL)
		state = (LexerStateFn)state(&lexer);

	return lexer.tokens;
}

bool check_illegal_tokens(const List* const tokens)
{
	assert(tokens != NULL);

	bool result = false;

	for list_range(it, *tokens) {
		Token* token = list_node_data(it, Token);

		if (token->type == TokenType_Illegal) {
			result = true;

			fputs("Error: illegal character \'", stderr);
			string_debug_print(&token->content);
			fprintf(stderr, "\' encountered at %lu\n", token->position);
		}
	}

	return result;
}

void debug_print_tokens(const List* const tokens)
{
	assert(tokens != NULL);

	putc('[', stderr);
	for list_range(it, *tokens) {
		Token* token = list_node_data(it, Token);

		string_debug_print(&token->content);

		if (it->next != NULL)
			fputs(", ", stderr);
	}
	fputs("]\n", stderr);
}

bool token_type_is_literal(const TokenType type)
{
	assert(0 <= type && type < TokenType__count);
	return TokenType__literals_begin < type && type < TokenType__literals_end;
}

bool token_type_is_operator(const TokenType type)
{
	assert(0 <= type && type < TokenType__count);
	return TokenType__operators_begin < type && type < TokenType__operators_end;
}

bool token_type_is_binary_operator(const TokenType type)
{
	assert(0 <= type && type < TokenType__count);
	return TokenType_Plus <= type && type <= TokenType_Exponent;
}

bool token_type_is_unary_operator(const TokenType type)
{
	assert(0 <= type && type < TokenType__count);
	return type == TokenType_Plus || type == TokenType_Minus;
}

bool token_type_is_right_associative(const TokenType type)
{
	assert(0 <= type && type < TokenType__count);
	return type == TokenType_Exponent;
}

static LexerState lexer_scan_text(Lexer* const lexer)
{
	assert(lexer != NULL);

	const uint8_t c = lexer_next(lexer);

	if (isspace(c))
		lexer_ignore(lexer);
	else if (isdigit(c)) {
		lexer_backup(lexer);
		return (LexerState)lexer_scan_number;
	}
	else if (isalpha(c)) {
		lexer_backup(lexer);
		return (LexerState)lexer_scan_symbol;
	}
	else if (is_operator(c)) {
		TokenType type;

		switch (c) {
		case '+':
			type = TokenType_Plus;
			break;

		case '-':
			type = TokenType_Minus;
			break;

		case '*':
			type = TokenType_Multiply;
			break;

		case '/':
			type = TokenType_Divide;
			break;

		case '^':
			type = TokenType_Exponent;
			break;

		case '(':
			type = TokenType_LeftParen;
			break;

		case ')':
			type = TokenType_RightParen;
			break;

		default:
			type = TokenType_Illegal;
			break;
		}

		lexer_emit(lexer, type);
	}
	else if (c == '\0')
		return NULL;
	else
		lexer_emit(lexer, TokenType_Illegal);

	return (LexerState)lexer_scan_text;
}

static LexerState lexer_scan_number(Lexer* const lexer)
{
	assert(lexer != NULL);

	while (isdigit(lexer_next(lexer)));
	lexer_backup(lexer);

	if (lexer_peek(lexer) == '.') {
		lexer_next(lexer);

		while (isdigit(lexer_next(lexer)));
		lexer_backup(lexer);
	}

	lexer_emit(lexer, TokenType_Number);

	return (LexerState)lexer_scan_text;
}

static LexerState lexer_scan_symbol(Lexer* const lexer)
{
	assert(lexer != NULL);

	uint8_t c = lexer_next(lexer);

	while (isalpha(c) || isdigit(c) || c == '_')
		c = lexer_next(lexer);

	lexer_backup(lexer);
	lexer_emit(lexer, TokenType_Symbol);

	return (LexerState)lexer_scan_text;
}

static uint8_t lexer_next(Lexer* const lexer)
{
	assert(lexer != NULL);

	if (lexer->position >= lexer->input->length) {
		lexer->stop = true;
		return '\0';
	}

	uint8_t result = lexer->input->text[lexer->position];
	++lexer->position;

	return result;
}

static void lexer_backup(Lexer* const lexer)
{
	assert(lexer != NULL);

	if (lexer->position > 0 && lexer->stop == false)
		--lexer->position;
}

static uint8_t lexer_peek(Lexer* const lexer)
{
	assert(lexer != NULL);

	uint8_t result = lexer_next(lexer);
	lexer_backup(lexer);

	return result;
}

static void lexer_ignore(Lexer* const lexer)
{
	assert(lexer != NULL);
	lexer->start = lexer->position;
}

static void lexer_emit(Lexer* const lexer, const TokenType type)
{
	assert(lexer != NULL);
	assert(0 <= type && type < TokenType__count);

	String content = string_trim(lexer->input, lexer->start, lexer->position);
	*list_insert_back(&lexer->tokens, Token) = (Token){
		.type = type,
		.position = lexer->start + 1,
		.content = content,
	};

	lexer->start = lexer->position;
}

static bool is_operator(const uint8_t c)
{
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' ||
	       c == '(' || c == ')';
}

