import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { CallbackQueueWriter } from
  '../../source/queues/callback_queue_writer';
import { PipeBrokenError } from '../../source/queues/pipe_broken_error';
import { QueueWriterPublisher } from
  '../../source/queues/queue_writer_publisher';

describe('QueueWriterPublisher', () => {
  it('push_to_single_monitor', () => {
    const publisher = new QueueWriterPublisher<number>();
    const values: number[] = [];
    publisher.monitor(new CallbackQueueWriter((v) => { values.push(v); }));
    publisher.push(1);
    publisher.push(2);
    assert.deepStrictEqual(values, [1, 2]);
  });

  it('push_to_multiple_monitors', () => {
    const publisher = new QueueWriterPublisher<number>();
    const a: number[] = [];
    const b: number[] = [];
    publisher.monitor(new CallbackQueueWriter((v) => { a.push(v); }));
    publisher.monitor(new CallbackQueueWriter((v) => { b.push(v); }));
    publisher.push(42);
    assert.deepStrictEqual(a, [42]);
    assert.deepStrictEqual(b, [42]);
  });

  it('push_with_no_monitors', () => {
    const publisher = new QueueWriterPublisher<number>();
    publisher.push(1);
  });

  it('removes_broken_monitor', () => {
    const publisher = new QueueWriterPublisher<number>();
    const values: number[] = [];
    const broken = new CallbackQueueWriter<number>(() => {});
    broken.close();
    publisher.monitor(broken);
    publisher.monitor(new CallbackQueueWriter((v) => { values.push(v); }));
    publisher.push(1);
    assert.deepStrictEqual(values, [1]);
  });

  it('close_breaks_monitors', () => {
    const publisher = new QueueWriterPublisher<number>();
    let breakError: Error = null;
    publisher.monitor(new CallbackQueueWriter(
      () => {},
      (error) => { breakError = error; }));
    publisher.close();
    assert.ok(breakError instanceof PipeBrokenError);
  });

  it('close_with_custom_error', () => {
    const publisher = new QueueWriterPublisher<number>();
    const error = new Error('custom');
    let breakError: Error = null;
    publisher.monitor(new CallbackQueueWriter(
      () => {},
      (e) => { breakError = e; }));
    publisher.close(error);
    assert.strictEqual(breakError, error);
  });

  it('close_is_idempotent', () => {
    const publisher = new QueueWriterPublisher<number>();
    let breakCount = 0;
    publisher.monitor(new CallbackQueueWriter(
      () => {},
      () => { ++breakCount; }));
    publisher.close();
    publisher.close();
    assert.strictEqual(breakCount, 1);
  });

  it('push_after_close_throws', () => {
    const publisher = new QueueWriterPublisher<number>();
    publisher.close();
    assert.throws(() => publisher.push(1), (error: Error) =>
      error instanceof PipeBrokenError);
  });

  it('monitor_after_close_immediately_closes', () => {
    const publisher = new QueueWriterPublisher<number>();
    publisher.close();
    let breakError: Error = null;
    publisher.monitor(new CallbackQueueWriter(
      () => {},
      (e) => { breakError = e; }));
    assert.ok(breakError instanceof PipeBrokenError);
  });
});
