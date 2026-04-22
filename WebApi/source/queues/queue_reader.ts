import { BaseQueue } from './base_queue';

/**
 * Interface for the read-only side of a Queue.
 * @param T - The data to read from the Queue.
 */
export interface QueueReader<T> extends BaseQueue {

  /**
   * Returns the first value in the queue and pops it, waiting until a
   * value is available.
   */
  pop(): Promise<T>;

  /**
   * Returns the first value in the queue if one is available and pops it
   * without waiting, otherwise returns null.
   */
  tryPop(): T;
}
