#include "parser.h"

#include <assert.h>
#include <stdbool.h>

#include "lexer.h"

static bool preprocess_tokens(List* const tokens);

Tree parse_expression(List* const tokens)
{
    assert(tokens != NULL);

    preprocess_tokens(tokens); 
    
    return Tree();
}

void print_expression(const Tree* const tree)
{
    assert(tree != NULL);
}

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
