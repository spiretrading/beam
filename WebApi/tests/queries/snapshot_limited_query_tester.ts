import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { SnapshotLimit } from '../../source/queries/snapshot_limit';
import { SnapshotLimitedQuery } from
  '../../source/queries/snapshot_limited_query';

describe('SnapshotLimitedQuery', () => {
  it('default_constructor', () => {
    assert.ok(new SnapshotLimitedQuery().snapshotLimit.equals(
      SnapshotLimit.NONE));
  });

  it('snapshot_limit', () => {
    const query = new SnapshotLimitedQuery();
    query.snapshotLimit = SnapshotLimit.fromTail(25);
    assert.ok(query.snapshotLimit.equals(SnapshotLimit.fromTail(25)));
  });

  it('equals', () => {
    const query = new SnapshotLimitedQuery(SnapshotLimit.fromHead(10));
    assert.ok(query.equals(new SnapshotLimitedQuery(
      SnapshotLimit.fromHead(10))));
    assert.ok(!query.equals(new SnapshotLimitedQuery(
      SnapshotLimit.fromTail(10))));
  });

  it('round_trip', () => {
    const query = new SnapshotLimitedQuery(SnapshotLimit.fromTail(25));
    const json = query.toJson();
    assert.deepStrictEqual(json.snapshot_limit, SnapshotLimit.fromTail(
      25).toJson());
    assert.ok(SnapshotLimitedQuery.fromJson(json).equals(query));
  });
});
