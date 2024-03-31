#ifndef __LEXER_H__
#define __LEXER_H__

#include <stddef.h>
#include <stdbool.h>

#include "string.h"
#include "list.h"

typedef enum token_type {
    TokenType_illegal,
    TokenType_error,

    TokenType__literals_begin,
    TokenType_number,        // [0-9]+(\.[0-9]+)?
    TokenType_symbol,        // [a-zA-Z][a-zA-Z0-9_]*
    TokenType__literals_end,

    TokenType__operators_begin,
    TokenType_add,           // +
    TokenType_subtract,      // -
    TokenType_multiply,      // *
    TokenType_divide,        // /
    TokenType_power,         // ^
    TokenType_left_paren,    // (
    TokenType_right_paren,   // )
    TokenType__operators_end,

    TokenType__count,
} TokenType;

typedef struct token {
    TokenType type;
    size_t position;
    String content;
} Token;

extern List lexical_scan(const String* const string);
extern bool check_scan_errors(const List* const tokens);
extern void debug_print_tokens(const List* const tokens);

extern bool token_type_is_literal(const TokenType type);
extern bool token_type_is_operator(const TokenType type);
extern bool token_type_is_binary_operator(const TokenType type);
extern bool token_type_is_unary_operator(const TokenType type);
extern bool token_type_is_right_associative(const TokenType type);

#endif // __LEXER_H__
