import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { Range } from '../../source/queries/range';
import { RangedQuery } from '../../source/queries/ranged_query';
import { Sequence } from '../../source/queries/sequence';

describe('RangedQuery', () => {
  it('default_constructor', () => {
    assert.ok(new RangedQuery().range.equals(Range.EMPTY));
  });

  it('range', () => {
    const query = new RangedQuery();
    query.range = Range.TOTAL;
    assert.ok(query.range.equals(Range.TOTAL));
  });

  it('equals', () => {
    assert.ok(new RangedQuery(Range.TOTAL).equals(new RangedQuery(Range.TOTAL)));
    assert.ok(!new RangedQuery(Range.TOTAL).equals(new RangedQuery()));
  });

  it('round_trip', () => {
    const query =
      new RangedQuery(new Range(new Sequence(1n), new Sequence(9n)));
    const json = query.toJson();
    assert.deepStrictEqual(json.range, query.range.toJson());
    assert.ok(RangedQuery.fromJson(json).equals(query));
  });
});
