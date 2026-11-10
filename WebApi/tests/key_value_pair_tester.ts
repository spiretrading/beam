import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { DirectoryEntry, KeyValuePair } from '../source';

describe('KeyValuePair', () => {
  it('equality_and_formatting', () => {
    const pair = new KeyValuePair(7, 'value');
    assert.ok(pair.equals(new KeyValuePair(7, 'value')));
    assert.ok(!pair.equals(new KeyValuePair(8, 'value')));
    assert.ok(!pair.equals(new KeyValuePair(7, 'other')));
    assert.equal(pair.toString(), '(7 value)');
  });

  it('serialization', () => {
    const pair = new KeyValuePair(
      DirectoryEntry.makeAccount(7, 'account'), 42);
    const json = pair.toJson();
    assert.deepEqual(json, {key: pair.key.toJson(), value: 42});
    assert.ok(KeyValuePair.fromJson<DirectoryEntry, number>(
      DirectoryEntry, Number, json).equals(pair));
  });
});
