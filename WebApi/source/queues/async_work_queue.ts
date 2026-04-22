import { CallbackQueue } from './callback_queue';
import { CallbackQueueWriter } from './callback_queue_writer';
import { PipeBrokenError } from './pipe_broken_error';

/** A work queue that automatically executes tasks via queueMicrotask. */
export class AsyncWorkQueue {

  /** Constructs an AsyncWorkQueue. */
  public constructor() {
    this._isClosed = false;
    this._tasks = [];
    this._callbacks = new CallbackQueue();
    this._isScheduled = false;
  }

  /**
   * Returns a slot.
   * @param callback - The callback when a new value is pushed.
   * @returns A queue that translates a push into an async work function.
   */
  public getSlot<T>(callback: (value: T) => void): CallbackQueueWriter<T>;

  /**
   * Returns a slot.
   * @param callback - The callback when a new value is pushed.
   * @param onBreak - The callback when the queue is broken.
   * @returns A queue that translates a push into an async work function.
   */
  public getSlot<T>(callback: (value: T) => void,
    onBreak: (error: Error) => void): CallbackQueueWriter<T>;

  public getSlot<T>(callback: (value: T) => void,
      onBreak?: (error: Error) => void): CallbackQueueWriter<T> {
    return this._callbacks.getSlot<T>(
      (value: T) => {
        this.enqueue(() => callback(value));
      },
      onBreak ?
        (error: Error) => {
          this.enqueue(() => onBreak(error));
        } : undefined);
  }

  /**
   * Adds a work function to the queue.
   * @param task - The work function to add.
   */
  public push(task: () => void): void {
    if(this._isClosed) {
      throw this._exception;
    }
    this.enqueue(task);
  }

  /** Breaks the AsyncWorkQueue, closing all slots. */
  public close(error?: Error): void {
    if(this._isClosed) {
      return;
    }
    this._isClosed = true;
    this._exception = error ?? new PipeBrokenError();
    this._callbacks.close(this._exception);
  }

  private enqueue(task: () => void): void {
    this._tasks.push(task);
    if(this._isScheduled) {
      return;
    }
    this._isScheduled = true;
    queueMicrotask(() => this.run());
  }

  private run(): void {
    while(this._tasks.length > 0) {
      this._tasks.shift()();
    }
    this._isScheduled = false;
  }

  private _isClosed: boolean;
  private _exception: Error;
  private _tasks: (() => void)[];
  private _callbacks: CallbackQueue;
  private _isScheduled: boolean;
}
