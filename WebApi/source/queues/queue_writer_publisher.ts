import { PipeBrokenError } from './pipe_broken_error';
import { Publisher } from './publisher';
import { QueueWriter } from './queue_writer';

/**
 * A Publisher that broadcasts pushes to multiple QueueWriters.
 * @param T - The type of data being published.
 */
export class QueueWriterPublisher<T> implements Publisher<T>, QueueWriter<T> {

  /** Constructs an empty QueueWriterPublisher. */
  public constructor() {
    this._queues = [];
    this._exception = null;
  }

  public push(value: T): void {
    if(this._exception) {
      throw this._exception;
    }
    this._queues = this._queues.filter((queue) => {
      try {
        queue.push(value);
        return true;
      } catch(e) {
        return false;
      }
    });
  }

  public monitor(queue: QueueWriter<T>): void {
    if(this._exception) {
      queue.close(this._exception);
      return;
    }
    this._queues.push(queue);
  }

  public close(error?: Error): void {
    if(this._exception) {
      return;
    }
    this._exception = error ?? new PipeBrokenError();
    for(const queue of this._queues) {
      queue.close(this._exception);
    }
    this._queues = [];
  }

  private _queues: QueueWriter<T>[];
  private _exception: Error;
}
