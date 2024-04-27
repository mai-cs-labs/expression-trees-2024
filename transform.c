#include "transform.h"

#include <assert.h>
#include <float.h>
#include <math.h>

#include "lexer.h"

// @NOTE: Put transformer functions prototypes here
static bool factor_difference_of_squares(Expression* const expression);
static bool fold_multipliers_to_diff_of_squares(Expression* const expression);
static bool multiply_numerator_and_denominator(Expression* const expression);

// Make deep copy of a given expression.
static Expression* expression_copy(const Expression* const expression);

// Compare two expressions, lhs and rhs and return true if two
// expressions are identical, otherwise - false.
static bool expression_equal(const Expression* const lhs,
                             const Expression* const rhs);

void simplify_expression(Expression* const expression)
{
	assert(expression != NULL);

	// @NOTE: Put simplification transformer functions here
	fold_multipliers_to_diff_of_squares(expression);
}

void expand_expression(Expression* const expression)
{
	assert(expression != NULL);

	// @NOTE: Put expander transformer functions here
	factor_difference_of_squares(expression);
	multiply_numerator_and_denominator(expression);
}

double evaluate_expression(const Expression* const expression)
{
	assert(expression != NULL);

	switch (expression->type) {
	case ExpressionType_Literal: {
		Literal* const literal = (Literal*)expression;

		if (literal->tag == LiteralTag_Number)
			return literal->number;
	} break;

	case ExpressionType_Unary: {
		UnaryExpression* const unary = (UnaryExpression*)expression;

		if (unary->operator == TokenType_Minus)
			return -evaluate_expression(unary->subexpression);

		return evaluate_expression(unary->subexpression);
	} break;

	case ExpressionType_Binary: {
		BinaryExpression* const binary = (BinaryExpression*)expression;

		switch (binary->operator) {
		case TokenType_Plus:
			return evaluate_expression(binary->left) +
			       evaluate_expression(binary->right);

		case TokenType_Minus:
			return evaluate_expression(binary->left) -
			       evaluate_expression(binary->right);

		case TokenType_Multiply:
			return evaluate_expression(binary->left) *
			       evaluate_expression(binary->right);

		case TokenType_Divide:
			return evaluate_expression(binary->left) /
			       evaluate_expression(binary->right);

		case TokenType_Exponent:
			return 0; //pow(evaluate_expression(binary->left),
			           //evaluate_expression(binary->right));
		}
	} break;
	}

	// @NOTE: ExpressionType_Empty and LiteralTag_Symbol case
	return 0;
}

//
// Expression transformers
//


// (a/b) * (c/d) => (a * c) / (b * d).
static bool multiply_numerator_and_denominator(Expression* const expression) {
    assert(expression != NULL);

    switch (expression->type) {
    case ExpressionType_Unary: {
        UnaryExpression* const unary = (UnaryExpression*)expression;
        return multiply_numerator_and_denominator(unary->subexpression);
    } break;

    case ExpressionType_Binary: {
        BinaryExpression* const binary = (BinaryExpression*)expression;

        bool other_result = multiply_numerator_and_denominator(binary->left) | multiply_numerator_and_denominator(binary->right);

        if (binary->operator == TokenType_Multiply) {
            Expression* a = NULL;
            Expression* b = NULL;
            Expression* c = NULL;
            Expression* d = NULL;
            bool lhs_is_fraction = false;
            bool rhs_is_fraction = false;
            
            if (binary->left->type == ExpressionType_Binary) {
                BinaryExpression* const lhs = (BinaryExpression*)binary->left;
                if (lhs->operator == TokenType_Divide) {
                    lhs_is_fraction = true;
                    a = lhs->left;
                    b = lhs->right;
                }
            }

            if (binary->right->type == ExpressionType_Binary) {
                BinaryExpression* const rhs = (BinaryExpression*)binary->right;
                if (rhs->operator == TokenType_Divide) {
                    rhs_is_fraction = true;
                    c = rhs->left;
                    d = rhs->right;
                }
            }
            
            	
            if (lhs_is_fraction && rhs_is_fraction) {
                binary->operator = TokenType_Divide;

                BinaryExpression* new_lhs = expression_binary_create(
                    TokenType_Multiply, expression_copy(a), expression_copy(c));
                new_lhs->base.parenthesised = true;

                BinaryExpression* new_rhs = expression_binary_create(
                    TokenType_Multiply, expression_copy(b), expression_copy(d));
                new_rhs->base.parenthesised = true;

                expression_destroy(&binary->left);
                expression_destroy(&binary->right);

                binary->left = (Expression*)new_lhs;
                binary->right = (Expression*)new_rhs;

		other_result = other_result | multiply_numerator_and_denominator(binary->left) | multiply_numerator_and_denominator(binary->right);
                return other_result;
            }  
        }

        return other_result;
    } break;
    }

    // Not found
    return false;
}

// Transform expression to factor differences of squares and return
// true if if differences of squared were found, false - otherwise.
//
// Examples: a^2 - b^2 -> (a - b) * (a + b),
//
//           (a_0 - 50)^2 - eps^2 -> (a_0 - 50 - eps) * (a_0 - 50 + eps),
//
//           (a^2 - b^2)^2 - (c^2 - d^2)^2 ->
//        -> ((a - b) * (a + b) - (c - d) * (c + d)) *
//         * ((a - b) * (a + b) + (c - d) * (c + d))
//
// The problem comes down to finding a subtree below, where A and B are
// expressions as well..
#if 0
       -
      / \
     /   \
    ^     ^    A^2 - A^B
   / \   / \
  A   2 B   2
#endif
// ..and tranform that subtree to this subtree below, where parentheses between
// the operator means, that expression must be parenthesised.
#if 0
       *
      / \
     /   \
   (-)   (+)  (A - B) * (A + B)
   / \   / \
  A   B A   B
#endif
// @NOTE: If statements were not flattened for relative clarity
static bool factor_difference_of_squares(Expression* const expression)
{
	assert(expression != NULL);

	switch (expression->type) {
	// Try to find difference of squares in unary expression
	case ExpressionType_Unary: {
		UnaryExpression* const unary = (UnaryExpression*)expression;
		return factor_difference_of_squares(unary->subexpression);
	} break;

	case ExpressionType_Binary: {
		BinaryExpression* const binary = (BinaryExpression*)expression;

		// Try to find difference of squares in the sides of binary expression first
		const bool other_result = factor_difference_of_squares(binary->left) |
		                          factor_difference_of_squares(binary->right);

		// First step, find _difference_ of two expressions
		if (binary->operator == TokenType_Minus) {
			Expression* a = NULL;
			Expression* b = NULL;
			bool lhs_pow_of_two = false;
			bool rhs_pow_of_two = false;

			// Left subexpression must be a binary operation
			if (binary->left->type == ExpressionType_Binary) {
				BinaryExpression* const lhs = (BinaryExpression*)binary->left;

				// and operator must be "raise to power of"
				if (lhs->operator == TokenType_Exponent) {
					// exponent must be a literal
					if (lhs->right->type == ExpressionType_Literal) {
						Literal* const number = (Literal*)lhs->right;

						// specifically, number literal
						if (number->tag == LiteralTag_Number) {
							// specifically, with value of 2
							if (fabs(number->number - 2.0) < DBL_EPSILON) {
								lhs_pow_of_two = true;
								a = lhs->left;
							}
						}
					}
				}
			}

			// Same as above, but for the right subexpression
			if (binary->right->type == ExpressionType_Binary) {
				BinaryExpression* const rhs = (BinaryExpression*)binary->right;

				if (rhs->operator == TokenType_Exponent) {
					if (rhs->right->type == ExpressionType_Literal) {
						Literal* const number = (Literal*)rhs->right;
						if (number->tag == LiteralTag_Number) {
							if (fabs(number->number - 2.0) < DBL_EPSILON) {
								rhs_pow_of_two = true;
								b = rhs->left;
							}
						}
					}
				}
			}

			// This is true when left and right differences
			// are expressions "raise to power of two" respectively
			if (lhs_pow_of_two && rhs_pow_of_two) {
				// Change operator from subtraction to multiplication
				binary->operator = TokenType_Multiply;

				// Remove parentheses from factors
				a->parenthesised = false;
				b->parenthesised = false;

				// Create new (A - B) expression, notice the copy
				BinaryExpression* new_lhs = expression_binary_create(
					TokenType_Minus, expression_copy(a), expression_copy(b));
				// A - B subexpression must be parenthesised
				new_lhs->base.parenthesised = true;

				// Create new (A + B) expression, notice the copy
				BinaryExpression* new_rhs = expression_binary_create(
					TokenType_Plus, expression_copy(a), expression_copy(b));
				// A + B subexpression must be parenthesised
				new_rhs->base.parenthesised = true;

				// Destroy old expressions
				expression_destroy(&binary->left);
				expression_destroy(&binary->right);

				// Assign the difference expression new left and right subexpressions
				binary->left = (Expression*)new_lhs;
				binary->right = (Expression*)new_rhs;

				return true;
			}
		}

		// Maybe the differences of squares were found deeper in the expression?
		return other_result;
	} break;
	}

	// Not found
	return false;
}

// Transform expression to fold multiplication of terms back to differences of
// squares and return true, if such subtree was found, false - otherwise.
//
// Examples: (a - b) * (a + b) -> a^2 - b^2,
//           (a - ((c - d) * (c + d))) * (a + ((c - d) * (c + d))) ->
//        -> a ^ 2 * (c ^ 2 * d ^ 2) ^ 2
static bool fold_multipliers_to_diff_of_squares(Expression* const expression)
{
	assert(expression != NULL);

	switch (expression->type) {
	case ExpressionType_Unary: {
		UnaryExpression* const unary = (UnaryExpression*)expression;
		return fold_multipliers_to_diff_of_squares(unary->subexpression);
	} break;

	case ExpressionType_Binary: {
		BinaryExpression* const binary = (BinaryExpression*)expression;

		const bool other_result = fold_multipliers_to_diff_of_squares(binary->left) |
		                          fold_multipliers_to_diff_of_squares(binary->right);

		BinaryExpression* lhs = NULL;
		BinaryExpression* rhs = NULL;

		// Must be multiplication of factors
		if (binary->operator != TokenType_Multiply)
			return other_result;

		// Factors must be binary operations
		if (binary->left->type != ExpressionType_Binary ||
		    binary->right->type != ExpressionType_Binary) {
			return other_result;
		}

		lhs = (BinaryExpression*)binary->left;
		rhs = (BinaryExpression*)binary->right;

		// Left factor must be a difference, right factor must be a sum
		if (lhs->operator != TokenType_Minus ||
		    rhs->operator != TokenType_Plus) {
			return other_result;
		}

		// Left and right factors must be parenthesised
		if (!lhs->base.parenthesised || !rhs->base.parenthesised)
			return other_result;

		// Subexpressions of factors must be the same
		if (!expression_equal(lhs->left, rhs->left) ||
		    !expression_equal(lhs->right, rhs->right)) {
			return other_result;
		}

		binary->operator = TokenType_Minus;

		BinaryExpression* const new_lhs = expression_binary_create(
			TokenType_Exponent,
			expression_copy(lhs->left),
			(Expression*)expression_literal_create_number(2)
		);

		BinaryExpression* const new_rhs = expression_binary_create(
			TokenType_Exponent,
			expression_copy(lhs->right),
			(Expression*)expression_literal_create_number(2)
		);

		expression_destroy((Expression**)&lhs);
		expression_destroy((Expression**)&rhs);

		binary->left = (Expression*)new_lhs;
		binary->right = (Expression*)new_rhs;

		return true;
	} break;
	}

	return false;
}

//
// Helper functions
//

static Expression* expression_copy(const Expression* const expression)
{
	assert(expression != NULL);

	Expression* result = NULL;

	switch (expression->type) {
	case ExpressionType_Literal: {
		Literal* const literal = (Literal*)expression;

		switch (literal->tag) {
		case LiteralTag_Number:
			result = (Expression*)expression_literal_create_number(literal->number);
			break;

		case LiteralTag_Symbol:
			result = (Expression*)expression_literal_create_symbol(&literal->symbol);
			break;
		}
	} break;

	case ExpressionType_Unary: {
		UnaryExpression* const unary = (UnaryExpression*)expression;
		result = (Expression*)expression_unary_create(
			unary->operator, expression_copy(unary->subexpression));
	} break;

	case ExpressionType_Binary: {
		BinaryExpression* const binary = (BinaryExpression*)expression;
		result = (Expression*)expression_binary_create(
			binary->operator,
			expression_copy(binary->left),
			expression_copy(binary->right));
	} break;
	}

	result->parenthesised = expression->parenthesised;

	return result;
}

static bool expression_equal(const Expression* const lhs,
                             const Expression* const rhs)
{
	assert(lhs != NULL);
	assert(rhs != NULL);

	if (lhs->type != rhs->type)
		return false;

	switch (lhs->type) {
	case ExpressionType_Literal: {
		Literal* const lhs_literal = (Literal*)lhs;
		Literal* const rhs_literal = (Literal*)rhs;

		if (lhs_literal->tag != rhs_literal->tag)
			return false;

		switch (lhs_literal->tag) {
		case LiteralTag_Number:
			if (fabs(lhs_literal->number - rhs_literal->number) < DBL_EPSILON)
				return true;
			break;

		case LiteralTag_Symbol:
			if (string_equal(&lhs_literal->symbol, &rhs_literal->symbol))
				return true;
			break;
		}
	} break;

	case ExpressionType_Unary: {
		UnaryExpression* const lhs_unary = (UnaryExpression*)lhs;
		UnaryExpression* const rhs_unary = (UnaryExpression*)rhs;

		if (lhs_unary->operator != rhs_unary->operator)
			return false;

		return expression_equal(lhs_unary->subexpression, rhs_unary->subexpression);
	} break;

	case ExpressionType_Binary: {
		BinaryExpression* const lhs_binary = (BinaryExpression*)lhs;
		BinaryExpression* const rhs_binary = (BinaryExpression*)rhs;

		if (lhs_binary->operator != rhs_binary->operator)
			return false;

		return expression_equal(lhs_binary->left, rhs_binary->left) &&
		       expression_equal(lhs_binary->right, rhs_binary->right);
	} break;
	}

	return false;
}
