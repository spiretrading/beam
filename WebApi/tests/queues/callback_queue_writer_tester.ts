import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { CallbackQueueWriter } from '../../source/queues/callback_queue_writer';
import { PipeBrokenError } from '../../source/queues/pipe_broken_error';

describe('CallbackQueueWriter', () => {
  it('push_invokes_callback', () => {
    const values: number[] = [];
    const queue = new CallbackQueueWriter<number>((value) => {
      values.push(value);
    });
    queue.push(1);
    queue.push(2);
    queue.push(3);
    assert.deepStrictEqual(values, [1, 2, 3]);
  });

  it('push_after_close_throws', () => {
    const queue = new CallbackQueueWriter<number>(() => {});
    queue.close();
    assert.throws(() => queue.push(1), (error: Error) =>
      error instanceof PipeBrokenError);
  });

  it('push_after_close_with_custom_error', () => {
    const queue = new CallbackQueueWriter<number>(() => {});
    const error = new Error('custom');
    queue.close(error);
    assert.throws(() => queue.push(1), (e: Error) => e === error);
  });

  it('close_invokes_on_break', () => {
    let breakError: Error = null;
    const queue = new CallbackQueueWriter<number>(
      () => {},
      (error) => { breakError = error; });
    queue.close();
    assert.ok(breakError instanceof PipeBrokenError);
  });

  it('close_invokes_on_break_with_custom_error', () => {
    let breakError: Error = null;
    const error = new Error('custom');
    const queue = new CallbackQueueWriter<number>(
      () => {},
      (e) => { breakError = e; });
    queue.close(error);
    assert.strictEqual(breakError, error);
  });

  it('close_is_idempotent', () => {
    let breakCount = 0;
    const queue = new CallbackQueueWriter<number>(
      () => {},
      () => { ++breakCount; });
    queue.close();
    queue.close();
    assert.strictEqual(breakCount, 1);
  });

  it('close_without_on_break', () => {
    const queue = new CallbackQueueWriter<number>(() => {});
    queue.close();
    assert.throws(() => queue.push(1));
  });

  it('close_nulls_callbacks', () => {
    const values: number[] = [];
    const queue = new CallbackQueueWriter<number>((value) => {
      values.push(value);
    });
    queue.push(1);
    queue.close();
    assert.deepStrictEqual(values, [1]);
    assert.throws(() => queue.push(2));
    assert.deepStrictEqual(values, [1]);
  });
});
