import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { BasicQuery } from '../../source/queries/basic_query';
import { ConstantExpression } from
  '../../source/queries/constant_expression';
import { InterruptionPolicy } from '../../source/queries/interruption_policy';
import { Range } from '../../source/queries/range';
import { Sequence } from '../../source/queries/sequence';
import { SnapshotLimit } from '../../source/queries/snapshot_limit';
import { IntValue, StringValue } from
  '../../source/queries/standard_values';
import { DirectoryEntry } from '../../source/service_locator/directory_entry';

describe('BasicQuery', () => {
  it('constructor', () => {
    const query = new BasicQuery<number>(5);
    assert.strictEqual(query.index, 5);
    assert.ok(query.range.equals(Range.EMPTY));
    assert.ok(query.snapshotLimit.equals(SnapshotLimit.NONE));
    assert.strictEqual(
      query.interruptionPolicy, InterruptionPolicy.BREAK_QUERY);
  });

  it('make_current_query', () => {
    const query = BasicQuery.makeCurrentQuery(5);
    assert.ok(query.range.equals(Range.TOTAL));
    assert.ok(query.snapshotLimit.equals(SnapshotLimit.fromTail(1)));
    assert.strictEqual(
      query.interruptionPolicy, InterruptionPolicy.IGNORE_CONTINUE);
  });

  it('make_latest_query', () => {
    const query = BasicQuery.makeLatestQuery(5);
    assert.ok(query.range.equals(Range.HISTORICAL));
    assert.ok(query.snapshotLimit.equals(SnapshotLimit.fromTail(1)));
    assert.strictEqual(
      query.interruptionPolicy, InterruptionPolicy.BREAK_QUERY);
  });

  it('make_real_time_query', () => {
    const query = BasicQuery.makeRealTimeQuery(5);
    assert.ok(query.range.equals(Range.REAL_TIME));
    assert.strictEqual(
      query.interruptionPolicy, InterruptionPolicy.IGNORE_CONTINUE);
  });

  it('to_string', () => {
    const query = new BasicQuery<number>(5);
    query.range = new Range(new Sequence(1n), new Sequence(9n));
    query.snapshotLimit = SnapshotLimit.fromTail(1);
    assert.strictEqual(
      query.toString(), '(5 (1 9) (TAIL 1) BREAK_QUERY true)');
  });

  it('sends_a_constant_filter', () => {
    const filter = new BasicQuery<number>(5).toJson().filter;
    assert.strictEqual(
      filter.expression.__type, 'Beam.Queries.ConstantExpression');
    assert.strictEqual(
      filter.expression.value.value.__type, 'Beam.Queries.BoolValue');
    assert.strictEqual(filter.expression.value.value.value, true);
  });

  it('round_trip', () => {
    const account = DirectoryEntry.makeAccount(100, 'trader');
    const query = BasicQuery.makeCurrentQuery(account);
    const json = query.toJson();
    assert.deepStrictEqual(json.index, account.toJson());
    assert.strictEqual(
      json.interruption_policy, InterruptionPolicy.IGNORE_CONTINUE);
    const restored = BasicQuery.fromJson<DirectoryEntry>(DirectoryEntry, json);
    assert.ok(restored.index.equals(account));
    assert.ok(restored.range.equals(query.range));
    assert.ok(restored.snapshotLimit.equals(query.snapshotLimit));
    assert.strictEqual(
      restored.interruptionPolicy, query.interruptionPolicy);
  });

  it('round_trips_a_filter', () => {
    const query = new BasicQuery<number>(5);
    query.filter = ConstantExpression.FALSE;
    const restored =
      BasicQuery.fromJson<number>(Number, query.toJson());
    assert.ok(restored.filter.equals(ConstantExpression.FALSE));
    assert.strictEqual(restored.toString(),
      '(5 Empty None BREAK_QUERY false)');
  });

  it('rejects_a_filter_that_is_not_boolean', () => {
    const query = new BasicQuery<number>(5);
    assert.throws(() => query.filter = new ConstantExpression(new IntValue(1)),
      TypeError);
    assert.throws(
      () => query.filter = new ConstantExpression(new StringValue('a')),
      TypeError);
    assert.ok(query.filter.equals(ConstantExpression.TRUE));
  });

  it('rejects_an_unregistered_filter', () => {
    const json = new BasicQuery<number>(5).toJson();
    json.filter = {expression: {__type: 'Beam.Queries.NotExpression'}};
    assert.throws(
      () => BasicQuery.fromJson<number>(Number, json), TypeError);
  });
});
