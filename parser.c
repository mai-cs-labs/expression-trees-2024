#include "parser.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "lexer.h"
#include "string.h"

#define LOG(message) fputs((message), stderr)
#define LOGF(format, ...) fprintf(stderr, (format), __VA_ARGS__)
#define LOGC(ch) fputc((ch), stderr)

typedef struct parser {
    List tokens;
} Parser;

static const String operator_string[AstOperator__count] = {
    [AstOperator_add]         = String("+"),
    [AstOperator_subtract]    = String("-"),
    [AstOperator_multiply]    = String("*"),
    [AstOperator_divide]      = String("/"),
    [AstOperator_raise_to]    = String("^"),
    [AstOperator_unary_plus]  = String("+"),
    [AstOperator_unary_minus] = String("-"),
};

static Ast* ast_create(const AstType type);
static void ast_clear(Ast* const ast);

static void ast__print(const Ast* const ast);

static Ast* parser_parse(Parser* const parser);

Ast* ast_parse(const List* const tokens)
{
    assert(tokens != NULL);

    Parser parser = {
        .tokens = *tokens,
    };

    return parser_parse(&parser);
}

void ast_destroy(Ast** const ast)
{
    assert(ast != NULL);
    ast_clear(*ast);
    *ast = NULL;
}

void ast_print(const Ast* const ast)
{
    assert(ast != NULL);
    ast__print(ast);
    putc('\n', stdout);
}

static Ast* ast_create(const AstType type)
{
    assert(0 <= type && type < AstType__count);

    Ast* const result = malloc(sizeof(Ast));
    if (result == NULL)
        return NULL;

    result->type = type;

    return result;
}

static void ast_clear(Ast* const ast)
{
    assert(ast != NULL);

    if (ast->type == AstType_unary_operator)
        ast_clear((Ast*)ast->unary.subexpression);
    else if (ast->type == AstType_binary_operator) {
        ast_clear((Ast*)ast->binary.left);
        ast_clear((Ast*)ast->binary.right);
    }

    free(ast);
}

static void ast__print(const Ast* const ast)
{
    assert(ast != NULL);

    switch (ast->type) {
    case AstType_empty: {
        fputs("()", stdout);
    } break;

    case AstType_literal: {
        const AstLiteral* const literal = &ast->literal;

        const bool parenthesised = literal->base.parenthesised;

        if (parenthesised)
            putc('(', stdout);

        switch (literal->type) {
        case AstLiteralType_number:
            fprintf(stdout, "%f", literal->number);
            break;

        case AstLiteralType_symbol:
            string_print(&literal->symbol);
            break;
        }

        if (parenthesised)
            putc(')', stdout);
    } break;

    case AstType_unary_operator: {
        const AstUnaryOperator* const unary = &ast->unary;
        fputs("-", stdout);
        ast__print((Ast*)unary->subexpression);
    } break;

    case AstType_binary_operator: {
        const AstBinaryOperator* const binary = &ast->binary;

        const bool parenthesised = binary->base.parenthesised;

        if (parenthesised)
            putc('(', stdout);

        ast__print((Ast*)binary->left);

        putc(' ', stdout);
        string_print(&operator_string[binary->operator]);
        putc(' ', stdout);

        ast__print((Ast*)binary->right);

        if (parenthesised)
            putc(')', stdout);
    } break;
    }
}

static Ast* parser_parse(Parser* const parser)
{
    assert(parser != NULL);
    return ast_create(AstType_empty);
}
