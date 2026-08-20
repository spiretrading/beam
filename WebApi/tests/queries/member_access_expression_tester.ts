import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { Expression } from '../../source/queries/expression';
import { MemberAccessExpression } from
  '../../source/queries/member_access_expression';
import { ParameterExpression } from
  '../../source/queries/parameter_expression';
import { QueryType } from '../../source/queries/query_type';

describe('MemberAccessExpression', () => {
  const REQUEST =
    new ParameterExpression(0, 'Nexus.AccountModificationRequest');

  it('constructor', () => {
    const expression =
      new MemberAccessExpression('id', QueryType.INT, REQUEST);
    assert.strictEqual(expression.name, 'id');
    assert.strictEqual(expression.type, QueryType.INT);
    assert.ok(expression.expression.equals(REQUEST));
  });

  it('equals', () => {
    const expression =
      new MemberAccessExpression('id', QueryType.INT, REQUEST);
    assert.ok(expression.equals(
      new MemberAccessExpression('id', QueryType.INT, REQUEST)));
    assert.ok(!expression.equals(
      new MemberAccessExpression('type', QueryType.INT, REQUEST)));
    assert.ok(!expression.equals(
      new MemberAccessExpression('id', QueryType.BOOL, REQUEST)));
    assert.ok(!expression.equals(new MemberAccessExpression(
      'id', QueryType.INT, new ParameterExpression(1, QueryType.INT))));
  });

  it('to_string', () => {
    assert.strictEqual(
      new MemberAccessExpression('id', QueryType.INT, REQUEST).toString(),
      '(parameter 0).id');
  });

  it('round_trip', () => {
    const expression =
      new MemberAccessExpression('id', QueryType.INT, REQUEST);
    const json = JSON.parse(JSON.stringify(expression.toJson()));
    assert.strictEqual(json.__type, 'Beam.Queries.MemberAccessExpression');
    assert.strictEqual(json.name, 'id');
    assert.strictEqual(json.type, 'int');
    assert.strictEqual(json.expression.expression.__type,
      'Beam.Queries.ParameterExpression');
    assert.ok(Expression.fromJson(json).equals(expression));
  });
});
