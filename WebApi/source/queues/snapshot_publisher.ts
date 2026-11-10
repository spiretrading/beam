import { Publisher } from './publisher';
import { QueueWriter } from './queue_writer';

/**
 * Publishes updates and provides a snapshot of the current state.
 * @param T - The type of update published.
 * @param S - The snapshot type.
 */
export interface SnapshotPublisher<T, S> extends Publisher<T> {

  /** Returns a snapshot copy, or null if unavailable. */
  getSnapshot(): S;

  /**
   * Sends the current state to a queue and subscribes it to later updates.
   * @param queue - The queue receiving the state and updates.
   */
  monitor(queue: QueueWriter<T>): void;

  /**
   * Captures a snapshot and subscribes a queue to subsequent updates.
   * @param queue - The queue receiving updates after the captured snapshot.
   * @param snapshot - Receives the snapshot copy, or null if unavailable.
   */
  monitor(queue: QueueWriter<T>, snapshot: { value: S }): void;
}
