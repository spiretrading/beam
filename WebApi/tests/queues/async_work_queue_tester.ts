import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { AsyncWorkQueue } from '../../source/queues/async_work_queue';
import { PipeBrokenError } from '../../source/queues/pipe_broken_error';

function nextMicrotask(): Promise<void> {
  return new Promise((resolve) => queueMicrotask(resolve));
}

describe('AsyncWorkQueue', () => {
  it('push_executes_asynchronously', async () => {
    const queue = new AsyncWorkQueue();
    let called = false;
    queue.push(() => { called = true; });
    assert.strictEqual(called, false);
    await nextMicrotask();
    assert.strictEqual(called, true);
  });

  it('slot_push_executes_asynchronously', async () => {
    const queue = new AsyncWorkQueue();
    const values: number[] = [];
    const slot = queue.getSlot<number>((value) => {
      values.push(value);
    });
    slot.push(1);
    slot.push(2);
    assert.deepStrictEqual(values, []);
    await nextMicrotask();
    assert.deepStrictEqual(values, [1, 2]);
  });

  it('multiple_slots', async () => {
    const queue = new AsyncWorkQueue();
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
    await nextMicrotask();
    assert.deepStrictEqual(numbers, [1, 2]);
    assert.deepStrictEqual(strings, ['a']);
  });

  it('close_breaks_slots', async () => {
    const queue = new AsyncWorkQueue();
    const slot = queue.getSlot<number>(() => {});
    queue.close();
    assert.throws(() => slot.push(1), (error: Error) =>
      error instanceof PipeBrokenError);
  });

  it('close_invokes_on_break_asynchronously', async () => {
    const queue = new AsyncWorkQueue();
    let breakError: Error = null;
    queue.getSlot<number>(
      () => {},
      (error) => { breakError = error; });
    queue.close();
    assert.strictEqual(breakError, null);
    await nextMicrotask();
    assert.ok(breakError instanceof PipeBrokenError);
  });

  it('close_with_custom_error', async () => {
    const queue = new AsyncWorkQueue();
    const error = new Error('custom');
    let breakError: Error = null;
    queue.getSlot<number>(
      () => {},
      (e) => { breakError = e; });
    queue.close(error);
    await nextMicrotask();
    assert.strictEqual(breakError, error);
  });

  it('close_is_idempotent', () => {
    const queue = new AsyncWorkQueue();
    queue.close();
    queue.close();
  });

  it('push_after_slot_batches_execution', async () => {
    const queue = new AsyncWorkQueue();
    const order: string[] = [];
    const slot = queue.getSlot<number>((value) => {
      order.push('slot:' + value);
    });
    slot.push(1);
    queue.push(() => { order.push('task'); });
    slot.push(2);
    assert.deepStrictEqual(order, []);
    await nextMicrotask();
    assert.deepStrictEqual(order, ['slot:1', 'task', 'slot:2']);
  });

  it('no_redundant_scheduling', async () => {
    const queue = new AsyncWorkQueue();
    const values: number[] = [];
    const slot = queue.getSlot<number>((value) => {
      values.push(value);
    });
    slot.push(1);
    slot.push(2);
    slot.push(3);
    await nextMicrotask();
    assert.deepStrictEqual(values, [1, 2, 3]);
  });

  it('push_after_drain_reschedules', async () => {
    const queue = new AsyncWorkQueue();
    const values: number[] = [];
    const slot = queue.getSlot<number>((value) => {
      values.push(value);
    });
    slot.push(1);
    await nextMicrotask();
    assert.deepStrictEqual(values, [1]);
    slot.push(2);
    assert.deepStrictEqual(values, [1]);
    await nextMicrotask();
    assert.deepStrictEqual(values, [1, 2]);
  });
});
