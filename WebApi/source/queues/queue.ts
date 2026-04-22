import { PipeBrokenError } from './pipe_broken_error';

/**
 * Implements a Queue that can wait for data to arrive.
 * @param T - The data to store in the Queue.
 */
export class Queue<T> {

  /** Constructs an empty Queue. */
  constructor() {
    this._queue = [];
    this._waiter = null;
    this._exception = null;
  }

  /** Returns whether this Queue is broken. */
  public isBroken(): boolean {
    return this._exception !== null && this._queue.length === 0;
  }

  public pop(): Promise<T> {
    if(this._queue.length > 0) {
      return Promise.resolve(this._queue.shift());
    }
    if(this._exception) {
      return Promise.reject(this._exception);
    }
    return new Promise((resolve, reject) => {
      this._waiter = { resolve, reject };
    });
  }

  public tryPop(): T {
    if(this._queue.length > 0) {
      return this._queue.shift();
    }
    return null;
  }

  public push(value: T): void {
    if(this._exception) {
      throw this._exception;
    }
    if(this._waiter) {
      const waiter = this._waiter;
      this._waiter = null;
      waiter.resolve(value);
    } else {
      this._queue.push(value);
    }
  }

  public close(error?: Error): void {
    if(this._exception) {
      return;
    }
    this._exception = error ?? new PipeBrokenError();
    if(this._waiter) {
      this._waiter.reject(this._exception);
      this._waiter = null;
    }
  }

  private _queue: T[];
  private _waiter: {
    resolve: (value: T) => void;
    reject: (error: Error) => void;
  };
  private _exception: Error;
}
