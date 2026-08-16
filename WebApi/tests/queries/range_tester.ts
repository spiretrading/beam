import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { DateTime } from '../../source/definitions/date_time';
import { Range } from '../../source/queries/range';
import { Sequence } from '../../source/queries/sequence';

describe('Range', () => {
  it('default_constructor', () => {
    assert.ok(new Range().equals(Range.EMPTY));
  });

  it('constants', () => {
    assert.ok((Range.TOTAL.start as Sequence).equals(Sequence.FIRST));
    assert.ok((Range.TOTAL.end as Sequence).equals(Sequence.LAST));
    assert.ok((Range.REAL_TIME.start as Sequence).equals(Sequence.PRESENT));
    assert.ok((Range.HISTORICAL.end as Sequence).equals(Sequence.PRESENT));
    assert.ok(!Range.HISTORICAL.equals(Range.TOTAL));
    assert.ok(!Range.REAL_TIME.equals(Range.TOTAL));
  });

  it('invalid_point', () => {
    const range = new Range(DateTime.NOT_A_DATE_TIME, Sequence.LAST);
    assert.ok(range.equals(Range.EMPTY));
  });

  it('infinity_maps_to_sequence', () => {
    const range = new Range(DateTime.NEG_INFIN, DateTime.POS_INFIN);
    assert.ok(range.equals(Range.TOTAL));
  });

  it('to_string', () => {
    assert.strictEqual(Range.EMPTY.toString(), 'Empty');
    assert.strictEqual(Range.TOTAL.toString(), 'Total');
    assert.strictEqual(
      new Range(new Sequence(1n), new Sequence(5n)).toString(), '(1 5)');
  });

  it('round_trip_sequence', () => {
    const range = new Range(new Sequence(10n), new Sequence(20n));
    const json = range.toJson();
    assert.deepStrictEqual(json.start, {which: 0, value: '10'});
    assert.deepStrictEqual(json.end, {which: 0, value: '20'});
    assert.ok(Range.fromJson(json).equals(range));
  });

  it('round_trip_preserves_the_constants', () => {
    assert.ok(Range.fromJson(Range.TOTAL.toJson()).equals(Range.TOTAL));
    assert.ok(
      Range.fromJson(Range.HISTORICAL.toJson()).equals(Range.HISTORICAL));
    assert.ok(
      Range.fromJson(Range.REAL_TIME.toJson()).equals(Range.REAL_TIME));
  });

  it('round_trip_date_time', () => {
    const start = DateTime.fromJson('20200101T120000');
    const end = DateTime.fromJson('20200102T120000');
    const range = new Range(start, end);
    const json = range.toJson();
    assert.strictEqual(json.start.which, 1);
    assert.strictEqual(json.end.which, 1);
    assert.ok(Range.fromJson(json).equals(range));
  });
});
