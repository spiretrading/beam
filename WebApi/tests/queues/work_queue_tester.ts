import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { WorkQueue } from '../../source/queues/work_queue';
import { PipeBrokenError } from '../../source/queues/pipe_broken_error';

describe('WorkQueue', () => {
  it('push_and_pop', async () => {
    const queue = new WorkQueue();
    let called = false;
    queue.push(() => { called = true; });
    const task = await queue.pop();
    assert.strictEqual(called, false);
    task();
    assert.strictEqual(called, true);
  });

  it('slot_push_enqueues_task', async () => {
    const queue = new WorkQueue();
    const values: number[] = [];
    const slot = queue.getSlot<number>((value) => {
      values.push(value);
    });
    slot.push(1);
    slot.push(2);
    assert.deepStrictEqual(values, []);
    (await queue.pop())();
    assert.deepStrictEqual(values, [1]);
    (await queue.pop())();
    assert.deepStrictEqual(values, [1, 2]);
  });

  it('slot_on_break_enqueues_task', async () => {
    const queue = new WorkQueue();
    let breakError: Error = null;
    queue.getSlot<number>(
      () => {},
      (error) => { breakError = error; });
    queue.close();
    assert.strictEqual(breakError, null);
    (await queue.pop())();
    assert.ok(breakError instanceof PipeBrokenError);
  });

  it('multiple_slots', async () => {
    const queue = new WorkQueue();
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
    (await queue.pop())();
    (await queue.pop())();
    (await queue.pop())();
    assert.deepStrictEqual(numbers, [1, 2]);
    assert.deepStrictEqual(strings, ['a']);
  });

  it('try_pop', () => {
    const queue = new WorkQueue();
    let called = false;
    queue.push(() => { called = true; });
    const task = queue.tryPop();
    assert.notStrictEqual(task, null);
    task();
    assert.strictEqual(called, true);
  });

  it('try_pop_empty', () => {
    const queue = new WorkQueue();
    assert.strictEqual(queue.tryPop(), null);
  });

  it('flush', () => {
    const queue = new WorkQueue();
    const values: number[] = [];
    const slot = queue.getSlot<number>((value) => {
      values.push(value);
    });
    slot.push(1);
    slot.push(2);
    slot.push(3);
    let task = queue.tryPop();
    while(task) {
      task();
      task = queue.tryPop();
    }
    assert.deepStrictEqual(values, [1, 2, 3]);
  });

  it('close_breaks_slots', () => {
    const queue = new WorkQueue();
    const slot = queue.getSlot<number>(() => {});
    queue.close();
    assert.throws(() => slot.push(1), (error: Error) =>
      error instanceof PipeBrokenError);
  });

  it('close_enqueues_final_task', async () => {
    const queue = new WorkQueue();
    queue.close();
    (await queue.pop())();
    await assert.rejects(queue.pop(), (error: Error) =>
      error instanceof PipeBrokenError);
  });

  it('close_is_idempotent', () => {
    const queue = new WorkQueue();
    queue.close();
    queue.close();
  });

  it('close_with_custom_error', async () => {
    const queue = new WorkQueue();
    const error = new Error('custom');
    queue.close(error);
    (await queue.pop())();
    await assert.rejects(queue.pop(), (e: Error) => e === error);
  });

  it('pop_waits_for_slot_push', async () => {
    const queue = new WorkQueue();
    const values: number[] = [];
    const slot = queue.getSlot<number>((value) => {
      values.push(value);
    });
    const promise = queue.pop();
    slot.push(42);
    (await promise)();
    assert.deepStrictEqual(values, [42]);
  });
});
