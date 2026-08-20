import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { OrExpression, disjunction } from
  '../../source/queries/or_expression';
import { ConstantExpression } from
  '../../source/queries/constant_expression';
import { Expression } from '../../source/queries/expression';
import { QueryType } from '../../source/queries/query_type';
import { IntValue } from '../../source/queries/standard_values';

describe('OrExpression', () => {
  it('constructor', () => {
    const expression =
      new OrExpression(ConstantExpression.TRUE, ConstantExpression.FALSE);
    assert.strictEqual(expression.type, QueryType.BOOL);
    assert.ok(expression.left.equals(ConstantExpression.TRUE));
    assert.ok(expression.right.equals(ConstantExpression.FALSE));
  });

  it('rejects_an_operand_that_is_not_boolean', () => {
    const value = new ConstantExpression(new IntValue(1));
    assert.throws(
      () => new OrExpression(value, ConstantExpression.TRUE), TypeError);
    assert.throws(
      () => new OrExpression(ConstantExpression.TRUE, value), TypeError);
  });

  it('equals', () => {
    const expression =
      new OrExpression(ConstantExpression.TRUE, ConstantExpression.FALSE);
    assert.ok(expression.equals(
      new OrExpression(ConstantExpression.TRUE, ConstantExpression.FALSE)));
    assert.ok(!expression.equals(
      new OrExpression(ConstantExpression.FALSE, ConstantExpression.TRUE)));
  });

  it('to_string', () => {
    assert.strictEqual(
      new OrExpression(
        ConstantExpression.TRUE, ConstantExpression.FALSE).toString(),
      '(or true false)');
  });

  it('round_trip', () => {
    const expression =
      new OrExpression(ConstantExpression.TRUE, ConstantExpression.FALSE);
    const json = JSON.parse(JSON.stringify(expression.toJson()));
    assert.strictEqual(json.__type, 'Beam.Queries.OrExpression');
    assert.strictEqual(
      json.left.expression.__type, 'Beam.Queries.ConstantExpression');
    assert.ok(Expression.fromJson(json).equals(expression));
  });

  it('disjunction', () => {
    assert.ok(disjunction([]).equals(ConstantExpression.FALSE));
    assert.ok(disjunction(
      [ConstantExpression.TRUE]).equals(ConstantExpression.TRUE));
    assert.ok(disjunction([ConstantExpression.TRUE, ConstantExpression.FALSE]).
      equals(new OrExpression(
        ConstantExpression.TRUE, ConstantExpression.FALSE)));
    assert.strictEqual(disjunction([ConstantExpression.TRUE,
      ConstantExpression.FALSE, ConstantExpression.TRUE]).toString(),
      '(or true (or false true))');
    assert.throws(
      () => disjunction([new ConstantExpression(new IntValue(1))]), TypeError);
  });
});
