import { QueueWriter } from './queue_writer';

/**
 * Interface for publishing values to multiple subscribers.
 * @param T - The type of data being published.
 */
export interface Publisher<T> {

  /**
   * Registers a queue to receive published values.
   * @param queue - The queue to push values onto.
   */
  monitor(queue: QueueWriter<T>): void;

  /**
   * Closes the publisher and all monitored queues.
   * @param error - The reason for closing.
   */
  close(error?: Error): void;
}
