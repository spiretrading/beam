import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { AndExpression, conjunction } from
  '../../source/queries/and_expression';
import { ConstantExpression } from
  '../../source/queries/constant_expression';
import { Expression } from '../../source/queries/expression';
import { QueryType } from '../../source/queries/query_type';
import { IntValue } from '../../source/queries/standard_values';

describe('AndExpression', () => {
  it('constructor', () => {
    const expression =
      new AndExpression(ConstantExpression.TRUE, ConstantExpression.FALSE);
    assert.strictEqual(expression.type, QueryType.BOOL);
    assert.ok(expression.left.equals(ConstantExpression.TRUE));
    assert.ok(expression.right.equals(ConstantExpression.FALSE));
  });

  it('rejects_an_operand_that_is_not_boolean', () => {
    const value = new ConstantExpression(new IntValue(1));
    assert.throws(
      () => new AndExpression(value, ConstantExpression.TRUE), TypeError);
    assert.throws(
      () => new AndExpression(ConstantExpression.TRUE, value), TypeError);
  });

  it('equals', () => {
    const expression =
      new AndExpression(ConstantExpression.TRUE, ConstantExpression.FALSE);
    assert.ok(expression.equals(
      new AndExpression(ConstantExpression.TRUE, ConstantExpression.FALSE)));
    assert.ok(!expression.equals(
      new AndExpression(ConstantExpression.FALSE, ConstantExpression.TRUE)));
  });

  it('to_string', () => {
    assert.strictEqual(
      new AndExpression(
        ConstantExpression.TRUE, ConstantExpression.FALSE).toString(),
      '(and true false)');
  });

  it('round_trip', () => {
    const expression =
      new AndExpression(ConstantExpression.TRUE, ConstantExpression.FALSE);
    const json = JSON.parse(JSON.stringify(expression.toJson()));
    assert.strictEqual(json.__type, 'Beam.Queries.AndExpression');
    assert.strictEqual(
      json.left.expression.__type, 'Beam.Queries.ConstantExpression');
    assert.ok(Expression.fromJson(json).equals(expression));
  });

  it('conjunction', () => {
    assert.ok(conjunction([]).equals(ConstantExpression.FALSE));
    assert.ok(conjunction(
      [ConstantExpression.TRUE]).equals(ConstantExpression.TRUE));
    assert.ok(conjunction([ConstantExpression.TRUE, ConstantExpression.FALSE]).
      equals(new AndExpression(
        ConstantExpression.TRUE, ConstantExpression.FALSE)));
    assert.strictEqual(conjunction([ConstantExpression.TRUE,
      ConstantExpression.FALSE, ConstantExpression.TRUE]).toString(),
      '(and true (and false true))');
    assert.throws(
      () => conjunction([new ConstantExpression(new IntValue(1))]), TypeError);
  });
});
