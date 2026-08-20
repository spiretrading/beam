import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { ConstantExpression } from
  '../../source/queries/constant_expression';
import { Expression } from '../../source/queries/expression';
import { NotExpression } from '../../source/queries/not_expression';
import { QueryType } from '../../source/queries/query_type';
import { IntValue } from '../../source/queries/standard_values';

describe('NotExpression', () => {
  it('constructor', () => {
    const expression = new NotExpression(ConstantExpression.TRUE);
    assert.strictEqual(expression.type, QueryType.BOOL);
    assert.ok(expression.operand.equals(ConstantExpression.TRUE));
  });

  it('rejects_an_operand_that_is_not_boolean', () => {
    assert.throws(
      () => new NotExpression(new ConstantExpression(new IntValue(1))),
      TypeError);
  });

  it('equals', () => {
    const expression = new NotExpression(ConstantExpression.TRUE);
    assert.ok(expression.equals(new NotExpression(ConstantExpression.TRUE)));
    assert.ok(!expression.equals(new NotExpression(ConstantExpression.FALSE)));
    assert.ok(!expression.equals(ConstantExpression.TRUE));
  });

  it('to_string', () => {
    assert.strictEqual(
      new NotExpression(ConstantExpression.FALSE).toString(), '(not false)');
  });

  it('round_trip', () => {
    const expression = new NotExpression(ConstantExpression.TRUE);
    const json = JSON.parse(JSON.stringify(expression.toJson()));
    assert.strictEqual(json.__type, 'Beam.Queries.NotExpression');
    assert.strictEqual(json.operand.expression.__type,
      'Beam.Queries.ConstantExpression');
    assert.ok(Expression.fromJson(json).equals(expression));
  });
});
