import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { QueryResult } from '../../source/queries/query_result';
import { DirectoryEntry } from '../../source/service_locator/directory_entry';

describe('QueryResult', () => {
  it('default_constructor', () => {
    const result = new QueryResult<number>();
    assert.strictEqual(result.id, -1);
    assert.deepStrictEqual(result.snapshot, []);
  });

  it('equals', () => {
    const result = new QueryResult<number>(5, [1, 2, 3]);
    assert.ok(result.equals(new QueryResult<number>(5, [1, 2, 3])));
    assert.ok(!result.equals(new QueryResult<number>(6, [1, 2, 3])));
    assert.ok(!result.equals(new QueryResult<number>(5, [1, 2])));
    assert.ok(!result.equals(new QueryResult<number>(5, [1, 2, 4])));
  });

  it('round_trip', () => {
    const accounts = [DirectoryEntry.makeAccount(100, 'trader'),
      DirectoryEntry.makeAccount(101, 'manager')];
    const result = new QueryResult(5, accounts);
    const json = result.toJson();
    assert.strictEqual(json.id, 5);
    assert.deepStrictEqual(json.snapshot,
      [accounts[0].toJson(), accounts[1].toJson()]);
    assert.ok(QueryResult.fromJson<DirectoryEntry>(
      DirectoryEntry, json).equals(result));
  });
});
