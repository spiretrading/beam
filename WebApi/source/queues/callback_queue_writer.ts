import { PipeBrokenError } from './pipe_broken_error';

/**
 * Used to invoke a callback when data is pushed onto this queue.
 * @param T - The type of data being pushed onto the queue.
 */
export class CallbackQueueWriter<T> {

  /**
   * Constructs a CallbackQueueWriter.
   * @param callback - The function to call when data is pushed onto this queue.
   * @param onBreak - The function to call when this queue is broken.
   */
  public constructor(
      callback: (value: T) => void, onBreak?: (error: Error) => void) {
    this._callback = callback;
    this._onBreak = onBreak ?? null;
    this._exception = null;
  }

  public push(value: T): void {
    if(this._exception) {
      throw this._exception;
    }
    this._callback(value);
  }

  public close(error?: Error): void {
    if(this._exception) {
      return;
    }
    this._exception = error ?? new PipeBrokenError();
    this._onBreak?.(this._exception);
    this._callback = null;
    this._onBreak = null;
  }

  private _callback: (value: T) => void;
  private _onBreak: (error: Error) => void;
  private _exception: Error;
}
