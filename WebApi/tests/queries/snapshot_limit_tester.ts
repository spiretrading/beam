import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { SnapshotLimit } from '../../source/queries/snapshot_limit';

describe('SnapshotLimit', () => {
  it('equals', () => {
    assert.ok(SnapshotLimit.fromHead(10).equals(SnapshotLimit.fromHead(10)));
    assert.ok(!SnapshotLimit.fromHead(10).equals(SnapshotLimit.fromTail(10)));
    assert.ok(!SnapshotLimit.fromHead(10).equals(SnapshotLimit.fromHead(11)));
  });

  it('equals_ignores_type_at_the_extremes', () => {
    assert.ok(SnapshotLimit.fromTail(0).equals(SnapshotLimit.NONE));
    assert.ok(SnapshotLimit.fromTail(2147483647).equals(
      SnapshotLimit.UNLIMITED));
  });

  it('to_string', () => {
    assert.strictEqual(SnapshotLimit.NONE.toString(), 'None');
    assert.strictEqual(SnapshotLimit.UNLIMITED.toString(), 'Unlimited');
    assert.strictEqual(SnapshotLimit.fromHead(5).toString(), '(HEAD 5)');
    assert.strictEqual(SnapshotLimit.fromTail(5).toString(), '(TAIL 5)');
  });

  it('round_trip', () => {
    const limit = SnapshotLimit.fromTail(25);
    const json = limit.toJson();
    assert.deepStrictEqual(json, {type: SnapshotLimit.Type.TAIL, size: 25});
    assert.ok(SnapshotLimit.fromJson(json).equals(limit));
  });
});
