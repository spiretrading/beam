import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { Expression } from '../../source/queries/expression';
import { ParameterExpression } from
  '../../source/queries/parameter_expression';
import { QueryType } from '../../source/queries/query_type';

describe('ParameterExpression', () => {
  it('constructor', () => {
    const expression = new ParameterExpression(1, QueryType.INT);
    assert.strictEqual(expression.index, 1);
    assert.strictEqual(expression.type, QueryType.INT);
  });

  it('equals', () => {
    const expression = new ParameterExpression(1, QueryType.INT);
    assert.ok(expression.equals(new ParameterExpression(1, QueryType.INT)));
    assert.ok(!expression.equals(new ParameterExpression(0, QueryType.INT)));
    assert.ok(!expression.equals(new ParameterExpression(1, QueryType.BOOL)));
  });

  it('to_string', () => {
    assert.strictEqual(
      new ParameterExpression(2, QueryType.INT).toString(), '(parameter 2)');
  });

  it('round_trip', () => {
    const expression = new ParameterExpression(1, QueryType.DATE_TIME);
    const json = JSON.parse(JSON.stringify(expression.toJson()));
    assert.deepStrictEqual(json, {
      __type: 'Beam.Queries.ParameterExpression',
      index: 1,
      type: 'boost.posix_time.ptime'
    });
    assert.ok(Expression.fromJson(json).equals(expression));
  });

  it('accepts_a_type_beam_does_not_register', () => {
    const expression = new ParameterExpression(0, 'Nexus.Money');
    assert.strictEqual(expression.type, 'Nexus.Money');
    assert.ok(Expression.fromJson(
      JSON.parse(JSON.stringify(expression.toJson()))).equals(expression));
  });
});
