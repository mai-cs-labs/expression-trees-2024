#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "string.h"
#include "list.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        LOGF("Usage: %s expression\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    String input = string_init(argv[1]);
    List tokens = lexical_scan(&input);
    Tree tree = parse_expression(&tokens);

    print_tokens(&tokens);
    print_expression(&tree); 

    list_deinit(&tokens);
}
