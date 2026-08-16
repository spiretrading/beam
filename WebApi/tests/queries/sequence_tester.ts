import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { Sequence } from '../../source/queries/sequence';

describe('Sequence', () => {
  it('default_constructor', () => {
    const sequence = new Sequence();
    assert.strictEqual(sequence.ordinal, 0);
    assert.ok(sequence.equals(Sequence.FIRST));
  });

  it('increment', () => {
    assert.strictEqual(new Sequence(5).increment().ordinal, 6);
    assert.ok(Sequence.LAST.increment().equals(Sequence.LAST));
  });

  it('decrement', () => {
    assert.strictEqual(new Sequence(5).decrement().ordinal, 4);
    assert.ok(Sequence.FIRST.decrement().equals(Sequence.FIRST));
  });

  it('compare', () => {
    assert.ok(new Sequence(1).compare(new Sequence(2)) < 0);
    assert.ok(new Sequence(2).compare(new Sequence(1)) > 0);
    assert.strictEqual(new Sequence(1).compare(new Sequence(1)), 0);
  });

  it('equals', () => {
    assert.ok(new Sequence(10).equals(new Sequence(10)));
    assert.ok(!new Sequence(10).equals(new Sequence(11)));
  });

  it('to_string', () => {
    assert.strictEqual(new Sequence(123).toString(), '123');
  });

  it('round_trip', () => {
    const sequence = new Sequence(42);
    assert.strictEqual(sequence.toJson(), 42);
    assert.ok(Sequence.fromJson(sequence.toJson()).equals(sequence));
  });
});
