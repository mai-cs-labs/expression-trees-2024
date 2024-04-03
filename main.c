#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "string.h"
#include "list.h"
#include "lexer.h"
#include "parser.h"
#include "transform.h"

static void print_short_usage(void)
{
    LOG("Usage: expr [-h|--help] [-v] [<command>] {expression}\n");
    exit(EXIT_SUCCESS);
}

static void print_long_usage(void)
{
    LOG("Usage: expr [-h|--help] [-v] [<command>] {expression}\n\n"
        "\t-h, --help\n"
        "\t\tOutput a usage message and exit\n\n"
        "\t-v\n"
        "\t\tEnable verbose expression output\n\n"
        "\tcommand, any of:\n"
        "\t\tsimplify\tsimplify resulting expression (default)\n"
        "\t\texpand\t\texpand resulting expression\n"
        "\t\teval\t\tevaluate resulting expression\n\n");
    exit(EXIT_SUCCESS);
}

static void print_expression(const Expression* const expression, const bool verbose)
{
    assert(expression != NULL);

    if (verbose)
        expression_verbose_print(expression);
    else
        expression_print(expression);
}

int main(int argc, char* argv[])
{
    int result = EXIT_SUCCESS;
    bool verbose = false;
    TransformMode transform = TransformMode_Simplify;

    if (argc == 1)
        print_short_usage();

    int argp = 1;
    while (argp != argc) {
        if (strcmp(argv[argp], "-v") == 0) {
            verbose = true;
            ++argp;
        } else if (strcmp(argv[argp], "-h") == 0) {
            print_short_usage();
        } else if (strcmp(argv[argp], "--help") == 0)  {
            print_long_usage();
        } else
            break;
    }

    if (argv[argp] == NULL)
        print_short_usage();
    else if (strcmp(argv[argp], "simplify") == 0) {
        transform = TransformMode_Simplify;
        ++argp;
    }
    else if (strcmp(argv[argp], "expand") == 0) {
        transform = TransformMode_Expand;
        ++argp;
    }
    else if (strcmp(argv[argp], "eval") == 0) {
        transform = TransformMode_Evaluate;
        ++argp;
    }

    if (argv[argp] == NULL)
        print_short_usage();

    String input = string_init(argv[argp]);
    List tokens = lexical_scan(&input);

    if (check_illegal_tokens(&tokens)) {
        result = EXIT_FAILURE;
        goto error_scan;
    }

    if (verbose)
        debug_print_tokens(&tokens);

    Expression* expression = expression_parse(&tokens);
    if (expression == NULL) {
        result = EXIT_FAILURE;
        goto error_scan;
    }

    if (expression_empty(expression))
        goto cleanup;

    switch (transform) {
    case TransformMode_Simplify:
        simplify_expression(expression);
        print_expression(expression, verbose);
        break;

    case TransformMode_Expand:
        expand_expression(expression);
        print_expression(expression, verbose);
        break;

    case TransformMode_Evaluate:
        printf("%f\n", evaluate_expression(expression));
        break;
    }

cleanup:
    expression_destroy(&expression);
error_scan:
    list_deinit(&tokens);

    return result;
}
