import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { IndexedValue } from '../../source/queries/indexed_value';
import { DirectoryEntry } from '../../source/service_locator/directory_entry';

describe('IndexedValue', () => {
  it('accessors', () => {
    const value = new IndexedValue<number, string>(42, 'TSX');
    assert.strictEqual(value.value, 42);
    assert.strictEqual(value.index, 'TSX');
  });

  it('equals', () => {
    const value = new IndexedValue<number, string>(42, 'TSX');
    assert.ok(value.equals(new IndexedValue<number, string>(42, 'TSX')));
    assert.ok(!value.equals(new IndexedValue<number, string>(42, 'NYSE')));
    assert.ok(!value.equals(new IndexedValue<number, string>(43, 'TSX')));
  });

  it('to_string', () => {
    const value = new IndexedValue<number, string>(42, 'TSX');
    assert.strictEqual(value.toString(), '(TSX 42)');
  });

  it('round_trip', () => {
    const account = DirectoryEntry.makeAccount(100, 'trader');
    const value = new IndexedValue<number, DirectoryEntry>(42, account);
    const json = value.toJson();
    assert.strictEqual(json.value, 42);
    assert.deepStrictEqual(json.index, account.toJson());
    assert.ok(IndexedValue.fromJson<number, DirectoryEntry>(
      Number, DirectoryEntry, json).equals(value));
  });
});
