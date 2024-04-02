#include "parser.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define LOG(message) fputs((message), stderr)
#define LOGF(format, ...) fprintf(stderr, (format), __VA_ARGS__)
#define LOGC(ch) fputc((ch), stderr)

typedef struct parser {
    List tokens;
} Parser;

static Expression* parser_parse(Parser* const parser);

static void expression_clear(Expression* const expression);
static Expression* expression__create(const ExpressionType tag, const size_t size);
static void expression__print(const Expression* const expression);

static Literal* expression_literal_create(void);
static UnaryExpression* expression_unary_create(void);
static BinaryExpression* expression_binary_create(void);

Expression* expression_create(const List* const tokens)
{
    assert(tokens != NULL);

    Parser parser = {
        .tokens = *tokens,
    };

    return parser_parse(&parser);
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

static Expression* parser_parse(Parser* const parser)
{
    assert(parser != NULL);
    return expression__create(ExpressionType_Empty, sizeof(Expression));
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

static Expression* expression__create(const ExpressionType type, const size_t size)
{
    assert(0 <= type && type < ExpressionType__count);
    assert(size > 0);

    Expression* const result = calloc(1, size);
    if (result == NULL)
        return NULL;

    result->type = type;

    return result;
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
            fprintf(stdout, "%f", literal->number);
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
        string_print(&unary->operator.content);
        expression__print(unary->subexpression);
    } break;

    case ExpressionType_Binary: {
        const BinaryExpression* const binary = (BinaryExpression*)expression;

        const bool parenthesised = binary->base.parenthesised;

        if (parenthesised)
            putc('(', stdout);

        expression__print(binary->left);

        putc(' ', stdout);
        string_print(&binary->operator.content);
        putc(' ', stdout);

        expression__print(binary->right);

        if (parenthesised)
            putc(')', stdout);
    } break;
    }
}

static Literal* expression_literal_create(void)
{
    return (Literal*)expression__create(ExpressionType_Literal,
                                        sizeof(Literal));
}

static UnaryExpression* expression_unary_create(void)
{
    return (UnaryExpression*)expression__create(ExpressionType_Unary,
                                                sizeof(UnaryExpression));
}

static BinaryExpression* expression_binary_create(void)
{
    return (BinaryExpression*)expression__create(ExpressionType_Binary,
                                                 sizeof(BinaryExpression));
}
