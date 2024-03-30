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

static void parser_parse(Parser* const parser);
static Expr* parser_parse_leaf(Parser* const parser);
static Expr* parser_parse_expression(Parser* const parser);
static Token* parser_next(Parser* const parser);

static void print_expression_inorder(Expr* const expr);
static bool preprocess_tokens(List* const tokens);

Tree parse_expression(List* const tokens)
{
    assert(tokens != NULL);

    Parser parser = {
        .input = *tokens,
        .expr = Tree(),
    };

    preprocess_tokens(tokens); 

    parser_parse(&parser);

    return parser.expr;
}

void print_expression(const Tree* const tree)
{
    assert(tree != NULL);

    print_expression_inorder((Expr*)tree->root);
}

static void print_expression_inorder(Expr* const expr)
{
    if (expr == NULL)
        return;

    print_expression_inorder((Expr*)expr->base.left);
    string_print(&expr->data.content);
    putchar(' ');
    print_expression_inorder((Expr*)expr->base.right);
}

static void parser_parse(Parser* const parser)
{
    assert(parser != NULL);
    parser->expr.root = (TreeNode*)parser_parse_expression(parser);
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

static Expr* parser_parse_expression(Parser* const parser)
{
    assert(parser != NULL);

    Expr* lhs = parser_parse_leaf(parser);
    if (lhs == NULL) {
        return NULL;
    }

    Token* operator = parser_next(parser);

    if (operator != NULL && token_type_is_operator(operator->type)) {
        Expr* rhs = parser_parse_expression(parser);
        if (rhs == NULL) {
            tree_node_destroy((TreeNode*)lhs);
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

#if 0
static Token* parser_peek(Parser* const parser)
{
    assert(parser != NULL);

    if (parser->input.head == NULL)
        return NULL;

    return list_node_data(parser->input.head, Token);
}
#endif

// When two numbers are together and the second number contains an operator
// at the beginning of it, then input was {"X", "+Y"} or {"X", "-Y"}, thus
// an operator token must be synthesised from it and operator symbol cut afterwards
static bool preprocess_tokens(List* const tokens)
{
    assert(tokens != NULL);

    for list_range(it, *tokens) {
        Token* token = list_node_data(it, Token);
        ListNodeBase* next_it = it->next;

        if (next_it == NULL)
            break;

        Token* next = list_node_data(next_it, Token);

        if (token->type == TokenType_number &&
            next->type == TokenType_number) {
            const uint8_t op = next->content.text[0];

            if (op == '+' || op == '-') {
                *list_insert_after(tokens, it, Token) = (Token){
                    .type = op == '+' ? TokenType_add : TokenType_subtract,
                    .content = string_trim(&next->content, 0, 1),
                };
                next->content = string_trim(&next->content, 1, next->content.length);
            } else {
                // TODO: No operator error
                return false;
            }
        }
    }

    return true;
}
