import { BaseQueue } from './base_queue';

/**
 * Interface for the write-side of a Queue.
 * @param T - The data to write to the Queue.
 */
export interface QueueWriter<T> extends BaseQueue {

  /**
   * Adds a value to the end of the Queue.
   * @param value - The value to add to the end of the Queue.
   */
  push(value: T): void;
}
