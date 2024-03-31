#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdbool.h>

#include "tree.h"
#include "list.h"

extern Tree parse_expression(List* const tokens);
extern void print_expression(const Tree* const tree);
extern void debug_print_expression(const Tree* const tree);

#endif // __PARSER_H__
