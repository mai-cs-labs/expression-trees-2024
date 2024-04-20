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
	LOG("Usage: expr [-h|--help] [-v] [-f <file>] [<command>] {expression}\n");
	exit(EXIT_SUCCESS);
}

static void print_long_usage(void)
{
	LOG("Usage: expr [-h|--help] [-v] [<command>] {expression}\n\n"
		"\t-h, --help\n"
		"\t\tOutput a usage message and exit\n\n"
		"\t-v\n"
		"\t\tEnable verbose expression output\n\n"
		"\t-f <file>\n"
		"\t\tRead expression from file\n\n"
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

	String input;

	int argp = 1;
	char* filename = NULL;

	if (argc > 1) {
		while (argp != argc) {
			if (strcmp(argv[argp], "-v") == 0) {
				verbose = true;
				++argp;
			}
			else if (strcmp(argv[argp], "-h") == 0)
				print_short_usage();
			else if (strcmp(argv[argp], "--help") == 0)
				print_long_usage();
			else if (strcmp(argv[argp], "-f") == 0) {
				filename = argv[argp + 1];
				argp += 2;
			}
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
			else
				break;
		}
	}
	else
		print_short_usage();

	if (filename != NULL) { // @NOTE: Expression provided as file
		FILE* const file = fopen(filename, "r");
		if (file == NULL) {
			LOGF("Failed to open file %s\n", filename);
			return EXIT_FAILURE;
		}

		fseek(file, 0, SEEK_END);
		input = string_create(ftell(file));
		rewind(file);

		if (fread(input.text, sizeof(uint8_t), input.length, file) != input.length) {
			LOGF("Failed to read file %s\n", filename);
			string_destroy(&input);
			fclose(file);
			return EXIT_FAILURE;
		}

		fclose(file);
	}
	else if (argv[argp] != NULL) // @NOTE: Expression provided as argument
		input = string_init(argv[argp]);
	else
		print_short_usage();

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
		printf("%.12g\n", evaluate_expression(expression));
		break;
	}

cleanup:
	expression_destroy(&expression);
error_scan:
	list_deinit(&tokens);

	string_destroy(&input);

	return result;
}
