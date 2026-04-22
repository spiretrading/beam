import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { CallbackQueue } from '../../source/queues/callback_queue';
import { PipeBrokenError } from '../../source/queues/pipe_broken_error';

describe('CallbackQueue', () => {
  it('get_slot_and_push', () => {
    const queue = new CallbackQueue();
    const values: number[] = [];
    const slot = queue.getSlot<number>((value) => {
      values.push(value);
    });
    slot.push(1);
    slot.push(2);
    assert.deepStrictEqual(values, [1, 2]);
  });

  it('multiple_slots', () => {
    const queue = new CallbackQueue();
    const numbers: number[] = [];
    const strings: string[] = [];
    const numberSlot = queue.getSlot<number>((value) => {
      numbers.push(value);
    });
    const stringSlot = queue.getSlot<string>((value) => {
      strings.push(value);
    });
    numberSlot.push(1);
    stringSlot.push('a');
    numberSlot.push(2);
    stringSlot.push('b');
    assert.deepStrictEqual(numbers, [1, 2]);
    assert.deepStrictEqual(strings, ['a', 'b']);
  });

  it('close_breaks_all_slots', () => {
    const queue = new CallbackQueue();
    const slot1 = queue.getSlot<number>(() => {});
    const slot2 = queue.getSlot<number>(() => {});
    queue.close();
    assert.throws(() => slot1.push(1), (error: Error) =>
      error instanceof PipeBrokenError);
    assert.throws(() => slot2.push(1), (error: Error) =>
      error instanceof PipeBrokenError);
  });

  it('close_with_custom_error', () => {
    const queue = new CallbackQueue();
    const error = new Error('custom');
    const slot = queue.getSlot<number>(() => {});
    queue.close(error);
    assert.throws(() => slot.push(1), (e: Error) => e === error);
  });

  it('close_invokes_on_break', () => {
    const queue = new CallbackQueue();
    let breakError: Error = null;
    queue.getSlot<number>(
      () => {},
      (error) => { breakError = error; });
    queue.close();
    assert.ok(breakError instanceof PipeBrokenError);
  });

  it('close_is_idempotent', () => {
    const queue = new CallbackQueue();
    let breakCount = 0;
    queue.getSlot<number>(
      () => {},
      () => { ++breakCount; });
    queue.close();
    queue.close();
    assert.strictEqual(breakCount, 1);
  });

  it('get_slot_after_close_is_immediately_broken', () => {
    const queue = new CallbackQueue();
    queue.close();
    const slot = queue.getSlot<number>(() => {});
    assert.throws(() => slot.push(1), (error: Error) =>
      error instanceof PipeBrokenError);
  });

  it('get_slot_after_close_invokes_on_break', () => {
    const queue = new CallbackQueue();
    const error = new Error('custom');
    queue.close(error);
    let breakError: Error = null;
    queue.getSlot<number>(
      () => {},
      (e) => { breakError = e; });
    assert.strictEqual(breakError, error);
  });

  it('push_calls_function', () => {
    const queue = new CallbackQueue();
    let called = false;
    queue.push(() => { called = true; });
    assert.strictEqual(called, true);
  });

  it('push_after_close_throws', () => {
    const queue = new CallbackQueue();
    queue.close();
    assert.throws(() => queue.push(() => {}), (error: Error) =>
      error instanceof PipeBrokenError);
  });
});
