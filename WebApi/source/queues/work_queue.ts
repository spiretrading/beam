import { CallbackQueue } from './callback_queue';
import { CallbackQueueWriter } from './callback_queue_writer';
import { PipeBrokenError } from './pipe_broken_error';
import { Queue } from './queue';

/** Used to translate queue pushes into work functions. */
export class WorkQueue {

  /** Constructs a WorkQueue. */
  public constructor() {
    this._isClosed = false;
    this._tasks = new Queue<() => void>();
    this._callbacks = new CallbackQueue();
  }

  /**
   * Returns a slot.
   * @param callback - The callback when a new value is pushed.
   * @returns A queue that translates a push into a work function.
   */
  public getSlot<T>(callback: (value: T) => void): CallbackQueueWriter<T>;

  /**
   * Returns a slot.
   * @param callback - The callback when a new value is pushed.
   * @param onBreak - The callback when the queue is broken.
   * @returns A queue that translates a push into a work function.
   */
  public getSlot<T>(callback: (value: T) => void,
    onBreak: (error: Error) => void): CallbackQueueWriter<T>;

  public getSlot<T>(callback: (value: T) => void,
      onBreak?: (error: Error) => void): CallbackQueueWriter<T> {
    return this._callbacks.getSlot<T>(
      (value: T) => {
        this._tasks.push(() => callback(value));
      },
      onBreak ?
        (error: Error) => {
          this._tasks.push(() => onBreak(error));
        } : undefined);
  }

  /**
   * Returns the next work function and pops it, waiting until one is
   * available.
   */
  public pop(): Promise<() => void> {
    return this._tasks.pop();
  }

  /**
   * Returns the next work function if one is available and pops it without
   * waiting, otherwise returns null.
   */
  public tryPop(): () => void {
    return this._tasks.tryPop();
  }

  /**
   * Adds a work function to the end of the queue.
   * @param task - The work function to add.
   */
  public push(task: () => void): void {
    this._tasks.push(task);
  }

  /** Breaks the WorkQueue, closing all slots. */
  public close(error?: Error): void {
    if(this._isClosed) {
      return;
    }
    this._isClosed = true;
    const exception = error ?? new PipeBrokenError();
    this._callbacks.close(exception);
    this._tasks.push(() => {
      this._tasks.close(exception);
    });
  }

  private _isClosed: boolean;
  private _tasks: Queue<() => void>;
  private _callbacks: CallbackQueue;
}
