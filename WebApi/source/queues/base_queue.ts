/** Base class for a Queue. */
export abstract class BaseQueue {

  /**
   * Breaks the Queue, indicating no further values will be published.
   * @param error - The reason why the Queue was broken.
   */
  abstract close(error?: Error): void;
}
