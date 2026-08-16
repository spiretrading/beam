import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { IndexedQuery } from '../../source/queries/indexed_query';
import { DirectoryEntry } from '../../source/service_locator/directory_entry';

describe('IndexedQuery', () => {
  it('index', () => {
    const query = new IndexedQuery<number>(5);
    assert.strictEqual(query.index, 5);
    query.index = 10;
    assert.strictEqual(query.index, 10);
  });

  it('equals', () => {
    const account = DirectoryEntry.makeAccount(100, 'trader');
    assert.ok(new IndexedQuery(account).equals(new IndexedQuery(account)));
    assert.ok(!new IndexedQuery(account).equals(
      new IndexedQuery(DirectoryEntry.makeAccount(101, 'trader'))));
  });

  it('round_trip', () => {
    const account = DirectoryEntry.makeAccount(100, 'trader');
    const query = new IndexedQuery(account);
    const json = query.toJson();
    assert.deepStrictEqual(json.index, account.toJson());
    assert.ok(
      IndexedQuery.fromJson<DirectoryEntry>(DirectoryEntry, json).equals(
        query));
  });
});
