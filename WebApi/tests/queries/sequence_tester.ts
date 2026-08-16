import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { Sequence } from '../../source/queries/sequence';

describe('Sequence', () => {
  it('default_constructor', () => {
    const sequence = new Sequence();
    assert.strictEqual(sequence.ordinal, 0n);
    assert.ok(sequence.equals(Sequence.FIRST));
  });

  it('increment', () => {
    assert.strictEqual(new Sequence(5n).increment().ordinal, 6n);
    assert.ok(Sequence.LAST.increment().equals(Sequence.LAST));
  });

  it('decrement', () => {
    assert.strictEqual(new Sequence(5n).decrement().ordinal, 4n);
    assert.ok(Sequence.FIRST.decrement().equals(Sequence.FIRST));
  });

  it('compare', () => {
    assert.ok(new Sequence(1n).compare(new Sequence(2n)) < 0);
    assert.ok(new Sequence(2n).compare(new Sequence(1n)) > 0);
    assert.strictEqual(new Sequence(1n).compare(new Sequence(1n)), 0);
  });

  it('equals', () => {
    assert.ok(new Sequence(10n).equals(new Sequence(10n)));
    assert.ok(!new Sequence(10n).equals(new Sequence(11n)));
  });

  it('the_extremes_stay_distinct', () => {
    assert.strictEqual(Sequence.LAST.ordinal, 18446744073709551615n);
    assert.ok(!Sequence.PRESENT.equals(Sequence.LAST));
    assert.strictEqual(Sequence.PRESENT.compare(Sequence.LAST), -1);
  });

  it('to_string', () => {
    assert.strictEqual(new Sequence(123n).toString(), '123');
  });

  it('round_trip', () => {
    const sequence = new Sequence(42n);
    assert.strictEqual(sequence.toJson(), '42');
    assert.ok(Sequence.fromJson(sequence.toJson()).equals(sequence));
  });

  it('round_trip_preserves_the_last_ordinal', () => {
    assert.strictEqual(Sequence.LAST.toJson(), '18446744073709551615');
    assert.ok(Sequence.fromJson(Sequence.LAST.toJson()).equals(Sequence.LAST));
    assert.ok(
      Sequence.fromJson(Sequence.PRESENT.toJson()).equals(Sequence.PRESENT));
  });
});
