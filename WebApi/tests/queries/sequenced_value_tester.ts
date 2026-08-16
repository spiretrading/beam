import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { Sequence } from '../../source/queries/sequence';
import { SequencedValue } from '../../source/queries/sequenced_value';
import { DirectoryEntry } from '../../source/service_locator/directory_entry';

describe('SequencedValue', () => {
  it('accessors', () => {
    const value = new SequencedValue<number>(42, new Sequence(7));
    assert.strictEqual(value.value, 42);
    assert.strictEqual(value.sequence.ordinal, 7);
  });

  it('equals', () => {
    const value = new SequencedValue<number>(42, new Sequence(7));
    assert.ok(value.equals(new SequencedValue<number>(42, new Sequence(7))));
    assert.ok(!value.equals(new SequencedValue<number>(42, new Sequence(8))));
    assert.ok(!value.equals(new SequencedValue<number>(43, new Sequence(7))));
  });

  it('to_string', () => {
    const value = new SequencedValue<number>(42, new Sequence(7));
    assert.strictEqual(value.toString(), '(42 7)');
  });

  it('round_trip', () => {
    const account = DirectoryEntry.makeAccount(100, 'trader');
    const value = new SequencedValue(account, new Sequence(7));
    const json = value.toJson();
    assert.deepStrictEqual(json.value, account.toJson());
    assert.strictEqual(json.sequence, 7);
    assert.ok(SequencedValue.fromJson<DirectoryEntry>(
      DirectoryEntry, json).equals(value));
  });
});
