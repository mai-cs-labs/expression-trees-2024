#include <stdio.h>

#include "string.h"
#include "list.h"
#include "lexer.h"

int main(void)
{
    String input = String("2112 14  0 +2 2 + 2 --14.2");
    
    List tokens = lexical_scan(&input);

    for list_range(it, tokens) {
        Token* token = list_node_data(it, Token);
        string_print(&token->content);
        putchar('\n');
    }

    list_deinit(&tokens);
}
