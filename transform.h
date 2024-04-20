#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include "parser.h"

typedef enum transform_mode {
	TransformMode_Simplify,
	TransformMode_Expand,
	TransformMode_Evaluate,
} TransformMode;

extern void simplify_expression(Expression* const expression);
extern void expand_expression(Expression* const expression);

extern double evaluate_expression(const Expression* const expression);

#endif // __TRANSFORM_H__
