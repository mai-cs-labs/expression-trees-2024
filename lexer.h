#ifndef __LEXER_H__
#define __LEXER_H__

#include "string.h"
#include "list.h"

typedef enum token_type {
    TokenType_eof,
    TokenType_number,
    TokenType_symbol,
    TokenType_operator,
    TokenType__count,
} TokenType;

typedef struct token {
    TokenType type;
    String content;
} Token;

extern List lexical_scan(const String* const string);

#endif // __LEXER_H__
