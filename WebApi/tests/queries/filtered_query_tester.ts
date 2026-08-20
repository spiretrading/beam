import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { ConstantExpression } from
  '../../source/queries/constant_expression';
import { checkFilter, FilteredQuery } from
  '../../source/queries/filtered_query';
import { IntValue } from '../../source/queries/standard_values';

describe('FilteredQuery', () => {
  it('constructor', () => {
    assert.ok(new FilteredQuery().filter.equals(ConstantExpression.TRUE));
    assert.ok(new FilteredQuery(ConstantExpression.FALSE).filter.equals(
      ConstantExpression.FALSE));
  });

  it('rejects_a_filter_that_is_not_boolean', () => {
    const filter = new ConstantExpression(new IntValue(1));
    assert.throws(() => new FilteredQuery(filter), TypeError);
    assert.throws(() => checkFilter(filter), TypeError);
    const query = new FilteredQuery();
    assert.throws(() => query.filter = filter, TypeError);
    assert.ok(query.filter.equals(ConstantExpression.TRUE));
  });

  it('equals', () => {
    const query = new FilteredQuery();
    assert.ok(query.equals(new FilteredQuery(ConstantExpression.TRUE)));
    assert.ok(!query.equals(new FilteredQuery(ConstantExpression.FALSE)));
    assert.ok(!query.equals(null));
  });

  it('to_string', () => {
    assert.strictEqual(new FilteredQuery().toString(), 'true');
    assert.strictEqual(
      new FilteredQuery(ConstantExpression.FALSE).toString(), 'false');
  });

  it('round_trip', () => {
    const query = new FilteredQuery(ConstantExpression.FALSE);
    const json = JSON.parse(JSON.stringify(query.toJson()));
    assert.strictEqual(
      json.filter.expression.__type, 'Beam.Queries.ConstantExpression');
    assert.ok(FilteredQuery.fromJson(json).equals(query));
  });
});
