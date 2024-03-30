#include "parser.h"

#include <assert.h>
#include <stdbool.h>

#include "lexer.h"
#include "tree.h"

typedef struct parser {
    List input;
    Tree expr;
} Parser;

typedef tree_node(Token) Expr;

static const size_t operator_precedence[] = {
    [TokenType_add] = 1,
    [TokenType_subtract] = 1,
    [TokenType_multiply] = 2,
    [TokenType_divide] = 2,
    [TokenType_power] = 3,
    [TokenType_left_paren] = 0,
    [TokenType_right_paren] = 0,
};

static void parser_parse(Parser* const parser);
static Expr* parser_parse_leaf(Parser* const parser);
static Expr* parser_parse_expression(Parser* const parser, const size_t precedence);
static Expr* parser_parse_binary(Parser* const parser,
                                 Expr* const lhs,
                                 const size_t precedence);

static Token* parser_next(Parser* const parser);
static void parser_backup(Parser* const parser);
// static Token* parser_peek(Parser* const parser);

static void print_expression_inorder(Expr* const expr, const bool verbose);

Tree parse_expression(List* const tokens)
{
    assert(tokens != NULL);

    Parser parser = {
        .input = *tokens,
        .expr = Tree(),
    };

    parser_parse(&parser);

    return parser.expr;
}

void print_expression(const Tree* const tree, const bool verbose)
{
    assert(tree != NULL);
    print_expression_inorder((Expr*)tree->root, verbose);
    putchar('\n');
}

static void parser_parse(Parser* const parser)
{
    assert(parser != NULL);
    parser->expr.root = (TreeNode*)parser_parse_expression(parser, 0);
}

static Expr* parser_parse_leaf(Parser* const parser)
{
    assert(parser != NULL);

    Token* token = parser_next(parser);
    if (token == NULL)
        return NULL;

    switch (token->type) {
    case TokenType_symbol:
    case TokenType_number:
        Expr* expr = tree_node_create(Expr);
        expr->data = *token;
        return expr;

    default:
        return NULL;
    }
}

static Expr* parser_parse_expression(Parser* const parser, const size_t precedence)
{
    assert(parser != NULL);

    Expr* lhs;

    Token* next = parser_next(parser);
    if (next == NULL)
        return NULL;
    
    if (next->type == TokenType_left_paren) {
        lhs = parser_parse_expression(parser, 0); 

        parser_backup(parser); 
        next = parser_next(parser); 

        if (next->type != TokenType_right_paren) {
            fputs("Expression was not closed, \')\' expected\n", stderr);
            return lhs;
        }
    } else {
        parser_backup(parser);
        lhs = parser_parse_leaf(parser);
    }

    if (lhs == NULL) {
        // TODO: Log error: leaf expected before operator
        return NULL;
    }

    for (;;) {
        Expr* binary = parser_parse_binary(parser, lhs, precedence);

        if (binary == NULL) {
            // TODO: Log error: expected leaf after operator
            break;
        }

        if (binary == lhs)
            break;

        parser_backup(parser);
        lhs = binary;
    }

    return lhs;
}

static Expr* parser_parse_binary(Parser* const parser,
                                 Expr* const lhs,
                                 const size_t precedence)
{
    assert(parser != NULL);
    assert(lhs != NULL);

    Token* operator = parser_next(parser);

    if (operator != NULL && token_type_is_binary_operator(operator->type)) {
        const size_t lhs_precedence = operator_precedence[operator->type];
        const size_t bias = token_type_is_right_associative(operator->type);

        if (lhs_precedence + bias > precedence) {
            Expr* rhs = parser_parse_expression(parser, lhs_precedence);
            if (rhs == NULL) {
                // TODO: Log error
                return NULL;
            }

            Expr* result = tree_node_create(Expr);
            if (result == NULL) {
                tree_node_destroy((TreeNode*)rhs);
                tree_node_destroy((TreeNode*)lhs);
                // TODO: Log error: failed to allocate binary expression
                return NULL;
            }

            result->data = *operator;
            result->base.left = (TreeNode*)lhs;
            result->base.right = (TreeNode*)rhs;

            return result;
        }
    }

    return lhs;
}

static Token* parser_next(Parser* const parser)
{
    assert(parser != NULL);

    if (parser->input.head == NULL)
        return NULL;
    
    Token* result = list_node_data(parser->input.head, Token);
    parser->input.head = parser->input.head->next;

    return result;
}

static void parser_backup(Parser* const parser)
{
    assert(parser != NULL);

    if (parser->input.head != NULL)
        parser->input.head = parser->input.head->prev;
    else if (parser->input.tail)
        parser->input.head = parser->input.tail;
}

#if 0
static Token* parser_peek(Parser* const parser)
{
    assert(parser != NULL);

    if (parser->input.head == NULL)
        return NULL;

    return list_node_data(parser->input.head, Token);
}
#endif

static void print_expression_inorder(Expr* const expr, const bool verbose)
{
    if (expr == NULL)
        return;

    const bool single = expr->base.left == NULL && expr->base.right == NULL;

    if (!single && verbose)
        putc('(', stdout);

    print_expression_inorder((Expr*)expr->base.left, verbose);

    if (expr->base.left != NULL)
        putchar(' ');

    string_print(&expr->data.content);

    if (expr->base.right != NULL)
        putchar(' ');

    print_expression_inorder((Expr*)expr->base.right, verbose);

    if (!single && verbose)
        putc(')', stdout);
}
