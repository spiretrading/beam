import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { ConstantExpression } from
  '../../source/queries/constant_expression';
import { ParameterExpression } from
  '../../source/queries/parameter_expression';
import { QueryType } from '../../source/queries/query_type';
import { ADDITION_NAME, DIVISION_NAME, EQUALS_NAME, GREATER_EQUALS_NAME,
  GREATER_NAME, LESS_EQUALS_NAME, LESS_NAME, MAX_NAME, MIN_NAME,
  MULTIPLICATION_NAME, NOT_EQUALS_NAME, SUBTRACTION_NAME, makeAddition,
  makeDivision, makeEquals, makeGreater, makeGreaterEquals, makeLess,
  makeLessEquals, makeMax, makeMin, makeMultiplication, makeNotEquals,
  makeSubtraction } from
  '../../source/queries/standard_function_expressions';
import { DecimalValue, IntValue } from '../../source/queries/standard_values';

describe('StandardFunctionExpressions', () => {
  const INTEGER = new ParameterExpression(0, QueryType.INT);
  const DECIMAL = new ConstantExpression(new DecimalValue(1.5));
  const FIVE = new ConstantExpression(new IntValue(5));

  it('comparisons_evaluate_to_a_boolean', () => {
    const comparisons: [any, string][] = [[makeLess, LESS_NAME],
      [makeLessEquals, LESS_EQUALS_NAME], [makeEquals, EQUALS_NAME],
      [makeNotEquals, NOT_EQUALS_NAME],
      [makeGreaterEquals, GREATER_EQUALS_NAME], [makeGreater, GREATER_NAME]];
    for(const [make, name] of comparisons) {
      const expression = make(INTEGER, FIVE);
      assert.strictEqual(expression.name, name);
      assert.strictEqual(expression.type, QueryType.BOOL);
      assert.strictEqual(expression.parameters.length, 2);
    }
  });

  it('arithmetic_evaluates_to_the_left_operand_type', () => {
    const operations: [any, string][] = [[makeAddition, ADDITION_NAME],
      [makeSubtraction, SUBTRACTION_NAME],
      [makeMultiplication, MULTIPLICATION_NAME], [makeDivision, DIVISION_NAME]];
    for(const [make, name] of operations) {
      assert.strictEqual(make(INTEGER, FIVE).name, name);
      assert.strictEqual(make(INTEGER, FIVE).type, QueryType.INT);
      assert.strictEqual(make(DECIMAL, FIVE).type, QueryType.DECIMAL);
    }
  });

  it('arithmetic_promotes_an_integer_against_a_decimal', () => {
    assert.strictEqual(
      makeAddition(INTEGER, DECIMAL).type, QueryType.DECIMAL);
    assert.strictEqual(
      makeDivision(INTEGER, DECIMAL).type, QueryType.DECIMAL);
  });

  it('max_and_min_evaluate_to_the_left_operand_type', () => {
    assert.strictEqual(makeMax(INTEGER, FIVE).name, MAX_NAME);
    assert.strictEqual(makeMax(INTEGER, FIVE).type, QueryType.INT);
    assert.strictEqual(makeMin(DECIMAL, FIVE).name, MIN_NAME);
    assert.strictEqual(makeMin(DECIMAL, FIVE).type, QueryType.DECIMAL);
  });

  it('to_string', () => {
    assert.strictEqual(
      makeGreaterEquals(INTEGER, FIVE).toString(), '(>= (parameter 0) 5)');
  });
});
