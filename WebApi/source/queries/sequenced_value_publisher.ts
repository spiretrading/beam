import { QueueWriter } from '../queues/queue_writer';
import { BasicQuery } from './basic_query';
import { Evaluator } from './evaluator';
import { testFilter } from './filtered_query';
import { InterruptionPolicy } from './interruption_policy';
import { QueryInterruptedError } from './query_interrupted_error';
import { Range } from './range';
import { Sequence } from './sequence';
import { SequencedValue } from './sequenced_value';

/**
 * Publishes sequenced query results across snapshots and live updates.
 * @param I - The query's index type.
 * @param V - The type of data received.
 */
export class SequencedValuePublisher<I, V> {

  /**
   * Constructs a SequencedValuePublisher.
   * @param query - The query submitted.
   * @param filter - The filter to apply to published values.
   * @param queue - The queue receiving the results.
   */
  constructor(query: BasicQuery<I>, filter: Evaluator<boolean>,
      queue: QueueWriter<SequencedValue<V>>) {
    this._query = query.clone();
    this._filter = filter;
    this._queue = queue;
    this._id = null;
    this._nextSequence = Sequence.FIRST;
    this._writeLog = [];
  }

  /** Returns the query id, or null while awaiting a snapshot. */
  public get id(): number {
    return this._id;
  }

  /**
   * Publishes a live value, buffering it while awaiting a snapshot.
   * @param value - The value to publish.
   */
  public push(value: SequencedValue<V>): void {
    if(this._id === null) {
      this._writeLog.push(value);
    } else {
      this.publish(value);
    }
  }

  /**
   * Publishes values from a query's snapshot.
   * @param snapshot - The snapshot values in sequence order.
   */
  public pushSnapshot(snapshot: Iterable<SequencedValue<V>>): void {
    for(const value of snapshot) {
      this.publish(value);
    }
  }

  /** Begins receiving a snapshot, buffering subsequent live updates. */
  public beginSnapshot(): void {
    this._id = null;
  }

  /**
   * Ends the snapshot and publishes buffered live updates.
   * @param id - The query's id, or null if no live query was established.
   */
  public endSnapshot(id: number): void {
    this._id = id;
    const writeLog = this._writeLog;
    this._writeLog = [];
    for(const value of writeLog) {
      this.publish(value);
    }
  }

  /**
   * Begins recovering an interrupted query, buffering live updates.
   * @return A query for recovering the missing data or resuming live updates.
   * @throws QueryInterruptedError If the policy requires breaking the query.
   */
  public beginRecovery(): BasicQuery<I> {
    this._id = null;
    let start: Range.Point;
    if(this._query.interruptionPolicy === InterruptionPolicy.IGNORE_CONTINUE) {
      start = Sequence.PRESENT;
    } else if(this._query.interruptionPolicy ===
        InterruptionPolicy.RECOVER_DATA) {
      if(this._nextSequence.equals(Sequence.FIRST)) {
        start = this._query.range.start;
      } else {
        start = this._nextSequence;
      }
    } else {
      const error = new QueryInterruptedError();
      this._queue.close(error);
      throw error;
    }
    const query = this._query.clone();
    query.range = new Range(start, this._query.range.end);
    return query;
  }

  /**
   * Ends recovery and resumes normal publication.
   * @param id - The resumed query's id.
   */
  public endRecovery(id: number): void {
    this.endSnapshot(id);
  }

  /**
   * Closes the queue receiving the published data.
   * @param error - The error used to close the queue.
   */
  public close(error?: Error): void {
    this._queue.close(error);
  }

  private publish(value: SequencedValue<V>): void {
    if(value.sequence.compare(this._nextSequence) >= 0 &&
        testFilter(this._filter, value.value)) {
      this._nextSequence = value.sequence.increment();
      this._queue.push(value);
    }
  }

  private _query: BasicQuery<I>;
  private _filter: Evaluator<boolean>;
  private _queue: QueueWriter<SequencedValue<V>>;
  private _id: number;
  private _nextSequence: Sequence;
  private _writeLog: SequencedValue<V>[];
}
