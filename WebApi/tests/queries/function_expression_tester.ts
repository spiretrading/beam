import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { ConstantExpression } from
  '../../source/queries/constant_expression';
import { Expression } from '../../source/queries/expression';
import { FunctionExpression } from
  '../../source/queries/function_expression';
import { ParameterExpression } from
  '../../source/queries/parameter_expression';
import { QueryType } from '../../source/queries/query_type';
import { IntValue } from '../../source/queries/standard_values';

describe('FunctionExpression', () => {
  const PARAMETER = new ParameterExpression(0, QueryType.INT);
  const FIVE = new ConstantExpression(new IntValue(5));

  it('constructor', () => {
    const expression =
      new FunctionExpression('==', QueryType.BOOL, [PARAMETER, FIVE]);
    assert.strictEqual(expression.name, '==');
    assert.strictEqual(expression.type, QueryType.BOOL);
    assert.strictEqual(expression.parameters.length, 2);
  });

  it('equals', () => {
    const expression =
      new FunctionExpression('==', QueryType.BOOL, [PARAMETER, FIVE]);
    assert.ok(expression.equals(
      new FunctionExpression('==', QueryType.BOOL, [PARAMETER, FIVE])));
    assert.ok(!expression.equals(
      new FunctionExpression('!=', QueryType.BOOL, [PARAMETER, FIVE])));
    assert.ok(!expression.equals(
      new FunctionExpression('==', QueryType.INT, [PARAMETER, FIVE])));
    assert.ok(!expression.equals(
      new FunctionExpression('==', QueryType.BOOL, [FIVE, PARAMETER])));
    assert.ok(!expression.equals(
      new FunctionExpression('==', QueryType.BOOL, [PARAMETER])));
  });

  it('to_string', () => {
    assert.strictEqual(
      new FunctionExpression('==', QueryType.BOOL, [PARAMETER, FIVE]).
        toString(), '(== (parameter 0) 5)');
    assert.strictEqual(
      new FunctionExpression('now', QueryType.DATE_TIME, []).toString(),
      '(now)');
  });

  it('round_trip', () => {
    const expression =
      new FunctionExpression('==', QueryType.BOOL, [PARAMETER, FIVE]);
    const json = JSON.parse(JSON.stringify(expression.toJson()));
    assert.strictEqual(json.__type, 'Beam.Queries.FunctionExpression');
    assert.strictEqual(json.name, '==');
    assert.strictEqual(json.type, 'bool');
    assert.strictEqual(json.parameters.length, 2);
    assert.strictEqual(json.parameters[0].expression.__type,
      'Beam.Queries.ParameterExpression');
    assert.ok(Expression.fromJson(json).equals(expression));
  });
});
