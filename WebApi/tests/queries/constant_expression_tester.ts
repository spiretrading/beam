import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { ConstantExpression } from
  '../../source/queries/constant_expression';
import { Expression } from '../../source/queries/expression';
import { QueryType } from '../../source/queries/query_type';
import { BoolValue, IntValue, StringValue } from
  '../../source/queries/standard_values';

describe('ConstantExpression', () => {
  it('constructor', () => {
    const expression = new ConstantExpression(new IntValue(5));
    assert.strictEqual(expression.type, QueryType.INT);
    assert.ok(expression.value.equals(new IntValue(5)));
  });

  it('constants', () => {
    assert.strictEqual(ConstantExpression.TRUE.type, QueryType.BOOL);
    assert.ok(ConstantExpression.TRUE.value.equals(new BoolValue(true)));
    assert.ok(ConstantExpression.FALSE.value.equals(new BoolValue(false)));
  });

  it('equals', () => {
    const expression = new ConstantExpression(new IntValue(5));
    assert.ok(expression.equals(new ConstantExpression(new IntValue(5))));
    assert.ok(!expression.equals(new ConstantExpression(new IntValue(6))));
    assert.ok(!expression.equals(ConstantExpression.TRUE));
  });

  it('to_string', () => {
    assert.strictEqual(ConstantExpression.TRUE.toString(), 'true');
    assert.strictEqual(
      new ConstantExpression(new StringValue('a')).toString(), '"a"');
  });

  it('round_trip', () => {
    const expression = new ConstantExpression(new IntValue(5));
    const json = JSON.parse(JSON.stringify(expression.toJson()));
    assert.strictEqual(json.__type, 'Beam.Queries.ConstantExpression');
    assert.strictEqual(json.value.value.__type, 'Beam.Queries.IntValue');
    assert.strictEqual(json.value.value.value, 5);
    const restored = Expression.fromJson(json);
    assert.ok(restored instanceof ConstantExpression);
    assert.ok(restored.equals(expression));
  });

  it('rejects_an_unregistered_expression', () => {
    assert.throws(
      () => Expression.fromJson({__type: 'Beam.Queries.NotExpression'}),
      TypeError);
  });
});
