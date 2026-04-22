import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { Queue } from '../../source/queues/queue';
import { PipeBrokenError } from '../../source/queues/pipe_broken_error';

describe('Queue', () => {
  it('push_and_pop', async () => {
    const queue = new Queue<number>();
    queue.push(1);
    assert.strictEqual(await queue.pop(), 1);
  });

  it('push_multiple_and_pop_in_order', async () => {
    const queue = new Queue<number>();
    queue.push(1);
    queue.push(2);
    queue.push(3);
    assert.strictEqual(await queue.pop(), 1);
    assert.strictEqual(await queue.pop(), 2);
    assert.strictEqual(await queue.pop(), 3);
  });

  it('pop_waits_for_push', async () => {
    const queue = new Queue<number>();
    const promise = queue.pop();
    queue.push(42);
    assert.strictEqual(await promise, 42);
  });

  it('try_pop', () => {
    const queue = new Queue<number>();
    queue.push(10);
    assert.strictEqual(queue.tryPop(), 10);
  });

  it('try_pop_empty', () => {
    const queue = new Queue<number>();
    assert.strictEqual(queue.tryPop(), null);
  });

  it('is_broken', () => {
    const queue = new Queue<number>();
    assert.strictEqual(queue.isBroken(), false);
    queue.close();
    assert.strictEqual(queue.isBroken(), true);
  });

  it('is_broken_with_pending_values', () => {
    const queue = new Queue<number>();
    queue.push(1);
    queue.close();
    assert.strictEqual(queue.isBroken(), false);
  });

  it('close_rejects_pending_pop', async () => {
    const queue = new Queue<number>();
    const promise = queue.pop();
    queue.close();
    await assert.rejects(promise, (error: Error) =>
      error instanceof PipeBrokenError);
  });

  it('close_with_custom_error', async () => {
    const queue = new Queue<number>();
    const promise = queue.pop();
    const error = new Error('custom');
    queue.close(error);
    await assert.rejects(promise, (e: Error) => e === error);
  });

  it('push_after_close_throws', () => {
    const queue = new Queue<number>();
    queue.close();
    assert.throws(() => queue.push(1), (error: Error) =>
      error instanceof PipeBrokenError);
  });

  it('pop_after_close_rejects', async () => {
    const queue = new Queue<number>();
    queue.close();
    await assert.rejects(queue.pop(), (error: Error) =>
      error instanceof PipeBrokenError);
  });

  it('close_is_idempotent', () => {
    const queue = new Queue<number>();
    queue.close();
    queue.close();
    assert.strictEqual(queue.isBroken(), true);
  });

  it('pop_remaining_after_close', async () => {
    const queue = new Queue<number>();
    queue.push(1);
    queue.push(2);
    queue.close();
    assert.strictEqual(await queue.pop(), 1);
    assert.strictEqual(await queue.pop(), 2);
    await assert.rejects(queue.pop(), (error: Error) =>
      error instanceof PipeBrokenError);
  });
});
