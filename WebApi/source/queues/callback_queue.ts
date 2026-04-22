import { CallbackQueueWriter } from './callback_queue_writer';
import { PipeBrokenError } from './pipe_broken_error';

/** Used to translate QueueWriter pushes into callbacks. */
export class CallbackQueue {

  /** Constructs a CallbackQueue. */
  public constructor() {
    this._exception = null;
    this._queues = [];
  }

  /**
   * Returns a slot.
   * @param callback - The callback when a new value is pushed.
   * @returns A queue that translates a push into a callback.
   */
  public getSlot<T>(callback: (value: T) => void): CallbackQueueWriter<T>;

  /**
   * Returns a slot.
   * @param callback - The callback when a new value is pushed.
   * @param onBreak - The callback when the queue is broken.
   * @returns A queue that translates a push into a callback.
   */
  public getSlot<T>(callback: (value: T) => void,
    onBreak: (error: Error) => void): CallbackQueueWriter<T>;

  public getSlot<T>(callback: (value: T) => void,
      onBreak?: (error: Error) => void): CallbackQueueWriter<T> {
    const queue = new CallbackQueueWriter<T>(callback, onBreak);
    if(this._exception) {
      queue.close(this._exception);
    } else {
      this._queues.push(queue);
    }
    return queue;
  }

  /**
   * Adds a function to be called.
   * @param callback - The function to call.
   */
  public push(callback: () => void): void {
    if(this._exception) {
      throw this._exception;
    }
    callback();
  }

  /** Breaks the CallbackQueue, closing all slots. */
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

  private _exception: Error;
  private _queues: CallbackQueueWriter<any>[];
}
