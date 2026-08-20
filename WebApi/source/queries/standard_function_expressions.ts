import { Expression } from './expression';
import { FunctionExpression } from './function_expression';
import { QueryType } from './query_type';

/** The name of the addition function. */
export const ADDITION_NAME = '+';

/** The name of the subtraction function. */
export const SUBTRACTION_NAME = '-';

/** The name of the multiplication function. */
export const MULTIPLICATION_NAME = '*';

/** The name of the division function. */
export const DIVISION_NAME = '/';

/** The name of the less than function. */
export const LESS_NAME = '<';

/** The name of the less than or equal function. */
export const LESS_EQUALS_NAME = '<=';

/** The name of the equality function. */
export const EQUALS_NAME = '==';

/** The name of the inequality function. */
export const NOT_EQUALS_NAME = '!=';

/** The name of the greater than or equal function. */
export const GREATER_EQUALS_NAME = '>=';

/** The name of the greater than function. */
export const GREATER_NAME = '>';

/** The name of the max function. */
export const MAX_NAME = 'max';

/** The name of the min function. */
export const MIN_NAME = 'min';

/**
 * Returns a FunctionExpression representing addition.
 * @param left - The left hand side of the expression.
 * @param right - The right hand side of the expression.
 */
export function makeAddition(
    left: Expression, right: Expression): FunctionExpression {
  return makeArithmetic(ADDITION_NAME, left, right);
}

/**
 * Returns a FunctionExpression representing subtraction.
 * @param left - The left hand side of the expression.
 * @param right - The right hand side of the expression.
 */
export function makeSubtraction(
    left: Expression, right: Expression): FunctionExpression {
  return makeArithmetic(SUBTRACTION_NAME, left, right);
}

/**
 * Returns a FunctionExpression representing multiplication.
 * @param left - The left hand side of the expression.
 * @param right - The right hand side of the expression.
 */
export function makeMultiplication(
    left: Expression, right: Expression): FunctionExpression {
  return makeArithmetic(MULTIPLICATION_NAME, left, right);
}

/**
 * Returns a FunctionExpression representing division.
 * @param left - The left hand side of the expression.
 * @param right - The right hand side of the expression.
 */
export function makeDivision(
    left: Expression, right: Expression): FunctionExpression {
  return makeArithmetic(DIVISION_NAME, left, right);
}

/**
 * Returns a FunctionExpression representing a less than comparison.
 * @param left - The left hand side of the expression.
 * @param right - The right hand side of the expression.
 */
export function makeLess(
    left: Expression, right: Expression): FunctionExpression {
  return makeComparison(LESS_NAME, left, right);
}

/**
 * Returns a FunctionExpression representing a less than or equal comparison.
 * @param left - The left hand side of the expression.
 * @param right - The right hand side of the expression.
 */
export function makeLessEquals(
    left: Expression, right: Expression): FunctionExpression {
  return makeComparison(LESS_EQUALS_NAME, left, right);
}

/**
 * Returns a FunctionExpression representing equality.
 * @param left - The left hand side of the expression.
 * @param right - The right hand side of the expression.
 */
export function makeEquals(
    left: Expression, right: Expression): FunctionExpression {
  return makeComparison(EQUALS_NAME, left, right);
}

/**
 * Returns a FunctionExpression representing inequality.
 * @param left - The left hand side of the expression.
 * @param right - The right hand side of the expression.
 */
export function makeNotEquals(
    left: Expression, right: Expression): FunctionExpression {
  return makeComparison(NOT_EQUALS_NAME, left, right);
}

/**
 * Returns a FunctionExpression representing a greater than or equal
 * comparison.
 * @param left - The left hand side of the expression.
 * @param right - The right hand side of the expression.
 */
export function makeGreaterEquals(
    left: Expression, right: Expression): FunctionExpression {
  return makeComparison(GREATER_EQUALS_NAME, left, right);
}

/**
 * Returns a FunctionExpression representing a greater than comparison.
 * @param left - The left hand side of the expression.
 * @param right - The right hand side of the expression.
 */
export function makeGreater(
    left: Expression, right: Expression): FunctionExpression {
  return makeComparison(GREATER_NAME, left, right);
}

/**
 * Returns a FunctionExpression representing the max function.
 * @param left - The left hand side of the expression.
 * @param right - The right hand side of the expression.
 */
export function makeMax(
    left: Expression, right: Expression): FunctionExpression {
  return new FunctionExpression(MAX_NAME, left.type, [left, right]);
}

/**
 * Returns a FunctionExpression representing the min function.
 * @param left - The left hand side of the expression.
 * @param right - The right hand side of the expression.
 */
export function makeMin(
    left: Expression, right: Expression): FunctionExpression {
  return new FunctionExpression(MIN_NAME, left.type, [left, right]);
}

function makeArithmetic(name: string, left: Expression, right: Expression):
    FunctionExpression {
  const type = (() => {
    if(left.type === QueryType.INT && right.type === QueryType.DECIMAL) {
      return QueryType.DECIMAL;
    }
    return left.type;
  })();
  return new FunctionExpression(name, type, [left, right]);
}

function makeComparison(name: string, left: Expression, right: Expression):
    FunctionExpression {
  return new FunctionExpression(name, QueryType.BOOL, [left, right]);
}
