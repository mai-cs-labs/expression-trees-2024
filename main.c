#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string.h"
#include "list.h"
#include "lexer.h"
#include "parser.h"

static bool verbose = false;
static bool dry_run = false;

static void print_short_usage(void)
{
    fputs("Usage: simplify [-h|--help] [-v] [-d] {expression}\n", stderr);
    exit(EXIT_SUCCESS);
}

static void print_long_usage(void)
{
    fputs("Usage: simplify [-h|--help] [-v] [-d] {expression}\n\n"
        "\t-h, --help\n"
        "\t\tOutput a usage message and exit\n\n"
        "\t-v\n"
        "\t\tEnable output of debug messages\n\n"
        "\t-d\n"
        "\t\tDry run: do not parse expression, implies -v\n", stderr);
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
    int result = EXIT_SUCCESS;

    if (argc == 1)
        print_short_usage();

    int argp = 1;
    while (argp != argc) {
        if (strcmp(argv[argp], "-v") == 0) {
            verbose = true;
            ++argp;
        } else if (strcmp(argv[argp], "-d") == 0) {
            dry_run = true;
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

    String input = string_init(argv[argp]);
    List tokens = lexical_scan(&input);

    if (check_illegal_tokens(&tokens)) {
        result = EXIT_FAILURE;
        goto error_scan;
    }

    debug_print_tokens(&tokens);

    Expression* expression = expression_create(&tokens);
    if (expression == NULL) {
        result = EXIT_FAILURE;
        goto error_scan;
    }

    expression_print(expression);

    expression_destroy(&expression);
error_scan:
    list_deinit(&tokens);

    return result;
}
