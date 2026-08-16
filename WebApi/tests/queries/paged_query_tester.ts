import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { PagedQuery } from '../../source/queries/paged_query';
import { SnapshotLimit } from '../../source/queries/snapshot_limit';
import { DirectoryEntry } from '../../source/service_locator/directory_entry';

describe('PagedQuery', () => {
  it('constructor', () => {
    const query = new PagedQuery<number, number>(5);
    assert.strictEqual(query.index, 5);
    assert.ok(query.snapshotLimit.equals(SnapshotLimit.NONE));
    assert.strictEqual(query.anchor, null);
    assert.strictEqual(query.offset, 0);
  });

  it('offset_is_clamped', () => {
    const query = new PagedQuery<number, number>(5);
    query.offset = 50;
    assert.strictEqual(query.offset, 50);
    query.offset = -1;
    assert.strictEqual(query.offset, 0);
  });

  it('to_string', () => {
    const query = new PagedQuery<number, number>(5);
    query.snapshotLimit = SnapshotLimit.fromHead(25);
    assert.strictEqual(query.toString(), '(5 (HEAD 25) true)');
    query.anchor = 17;
    query.offset = 50;
    assert.strictEqual(query.toString(), '(5 (HEAD 25) 17 50 true)');
  });

  it('sends_a_constant_filter', () => {
    const filter = new PagedQuery<number, number>(5).toJson().filter;
    assert.strictEqual(
      filter.expression.__type, 'Beam.Queries.ConstantExpression');
    assert.strictEqual(
      filter.expression.value.value.__type, 'Beam.Queries.BoolValue');
    assert.strictEqual(filter.expression.value.value.value, true);
  });

  it('round_trip_anchored', () => {
    const account = DirectoryEntry.makeAccount(100, 'trader');
    const query = new PagedQuery<DirectoryEntry, number>(account);
    query.snapshotLimit = SnapshotLimit.fromHead(25);
    query.anchor = 17;
    query.offset = 50;
    const json = query.toJson();
    assert.deepStrictEqual(json.anchor, {is_initialized: true, value: 17});
    assert.strictEqual(json.offset, 50);
    const restored =
      PagedQuery.fromJson<DirectoryEntry, number>(DirectoryEntry, Number, json);
    assert.ok(restored.index.equals(account));
    assert.ok(restored.snapshotLimit.equals(query.snapshotLimit));
    assert.strictEqual(restored.anchor, 17);
    assert.strictEqual(restored.offset, 50);
  });

  it('round_trip_without_an_anchor', () => {
    const query = new PagedQuery<number, number>(5);
    query.snapshotLimit = SnapshotLimit.fromTail(25);
    const json = query.toJson();
    assert.deepStrictEqual(json.anchor, {is_initialized: false});
    const restored =
      PagedQuery.fromJson<number, number>(Number, Number, json);
    assert.strictEqual(restored.anchor, null);
    assert.strictEqual(restored.offset, 0);
  });

  it('ignores_a_received_filter', () => {
    const json = new PagedQuery<number, number>(5).toJson();
    json.filter = {expression: {__type: 'Beam.Queries.NotExpression'}};
    assert.strictEqual(
      PagedQuery.fromJson<number, number>(Number, Number, json).index, 5);
  });
});
