#include "lexer.h"

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

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
static bool lexer_accept(Lexer* const lexer, const String* const valid);
static void lexer_accept_run(Lexer* const lexer, const String* const valid);
static void lexer_emit(Lexer* const lexer, const TokenType type);

static bool is_digit(const uint8_t c);
static bool is_operator(const uint8_t c);
static bool is_letter(const uint8_t c);

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
    while (state != NULL) {
        state = (LexerStateFn)state(&lexer);
    }

    return lexer.tokens;
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
    return TokenType_add <= type && type <= TokenType_power;
}

bool token_type_is_right_associative(const TokenType type)
{
    assert(0 <= type && type < TokenType__count);
    return type == TokenType_power;
}

void print_tokens(const List* const tokens)
{
    assert(tokens != NULL);

    putc('[', stderr);
    for list_range(it, *tokens) {
        Token* token = list_node_data(it, Token);
        const TokenType type = token->type;
        const String* content = &token->content;

        const char delim = token_type_is_literal(type) ? '\"' :
                           token_type_is_operator(type) ? '\'' :
                           '`';

        if (type == TokenType_illegal)
            string_debug_print(&String("Illegal:"));

        putc(delim, stderr);
        string_debug_print(content);
        putc(delim, stderr);

        if (it->next != NULL)
            fputs(", ", stderr);
    }
    fputs("]\n", stderr);
}

static LexerState lexer_scan_text(Lexer* const lexer)
{
    assert(lexer != NULL);

    const uint8_t c = lexer_next(lexer);

    if (c == '+' || c == '-' || is_digit(c)) {
        lexer_backup(lexer);
        return (LexerState)lexer_scan_number;
    }
    else if (is_letter(c)) {
        lexer_backup(lexer);
        return (LexerState)lexer_scan_symbol;
    }
    else if (is_operator(c)) {
        TokenType type;

        switch (c) {
        case '*':
            type = TokenType_multiply;
            break;

        case '/':
            type = TokenType_divide;
            break;

        case '^':
            type = TokenType_power;
            break;

        case '(':
            type = TokenType_left_paren;
            break;

        case ')':
            type = TokenType_right_paren;
            break;
        }

        lexer_emit(lexer, type);
    }

    else if (c == (uint8_t)EOF)
        return NULL;

    lexer_ignore(lexer);

    return (LexerState)lexer_scan_text;
}

// +
//  ^

static LexerState lexer_scan_number(Lexer* const lexer)
{
    assert(lexer != NULL);

    lexer_accept(lexer, &String("+-"));

    if (!is_digit(lexer_peek(lexer))) {
        lexer_backup(lexer); 

        uint8_t operator = lexer_next(lexer);
        lexer_emit(lexer, operator == '+' ? TokenType_add : TokenType_subtract);

        return (LexerState)lexer_scan_text;
    }

    const String digits = String("0123456789");
    lexer_accept_run(lexer, &digits); 

    if (lexer_accept(lexer, &String(".")))
        lexer_accept_run(lexer, &digits);

    if (lexer_accept(lexer, &String("eE")))
        lexer_accept_run(lexer, &digits);

    uint8_t next = lexer_peek(lexer);
    if (is_letter(next)) {
        lexer_next(lexer);
        lexer_ignore(lexer);
        // TODO: Return error
        return (LexerState)lexer_scan_text;
    }

    lexer_emit(lexer, TokenType_number);

    return (LexerState)lexer_scan_text; 
}

static LexerState lexer_scan_symbol(Lexer* const lexer)
{
    assert(lexer != NULL);

    uint8_t c = lexer_next(lexer);

    while (is_letter(c))
        c = lexer_next(lexer);
    
    lexer_backup(lexer);
    lexer_emit(lexer, TokenType_symbol);

    return (LexerState)lexer_scan_text;
}

static uint8_t lexer_next(Lexer* const lexer)
{
    assert(lexer != NULL);

    if (lexer->position >= lexer->input->length) {
        lexer->stop = true;
        return EOF;
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

static bool lexer_accept(Lexer* const lexer, const String* const valid)
{
    assert(lexer != NULL);
    assert(valid != NULL);

    if (string_index(valid, lexer_next(lexer)) < valid->length)
        return true;

    lexer_backup(lexer); 

    return false;
}

static void lexer_accept_run(Lexer* const lexer, const String* const valid)
{
    assert(lexer != NULL);
    assert(valid != NULL);

    while (string_index(valid, lexer_next(lexer)) < valid->length);

    lexer_backup(lexer); 
}

static void lexer_emit(Lexer* const lexer, const TokenType type)
{
    assert(lexer != NULL);
    assert(0 <= type && type < TokenType__count);

    String content = string_trim(lexer->input, lexer->start, lexer->position);
    *list_insert_back(&lexer->tokens, Token) = (Token){type, content};

    lexer->start = lexer->position;
}

static bool is_digit(const uint8_t c)
{
    return '0' <= c && c <= '9';
}

static bool is_operator(const uint8_t c)
{
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' ||
           c == '(' || c == ')';
}

static bool is_letter(const uint8_t c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

