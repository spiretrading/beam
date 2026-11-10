import { KeyValuePair } from '../key_value_pair';
import { Map } from '../map';
import { QueueWriter } from './queue_writer';
import { QueueWriterPublisher } from './queue_writer_publisher';
import { SnapshotPublisher } from './snapshot_publisher';

/**
 * Publishes updates to a table indexed using Beam's equality and hashing.
 * @param K - The unique key into the table.
 * @param V - The value associated with each key.
 */
export class TablePublisher<K, V> implements SnapshotPublisher<
    KeyValuePair<K, V>, Map<K, V>>, QueueWriter<KeyValuePair<K, V>> {

  /** Constructs an empty table publisher. */
  constructor() {
    this._table = new Map();
    this._publisher = new QueueWriterPublisher();
  }

  /**
   * Inserts or replaces an entry and publishes it.
   * @param entry - The key and value to store.
   */
  public push(entry: KeyValuePair<K, V>): void;

  /**
   * Inserts or replaces an entry and publishes it.
   * @param key - The entry's key.
   * @param value - The value to associate with the key.
   */
  public push(key: K, value: V): void;
  public push(entryOrKey: KeyValuePair<K, V> | K, value?: V): void {
    let entry: KeyValuePair<K, V>;
    if(arguments.length === 1) {
      entry = entryOrKey as KeyValuePair<K, V>;
    } else {
      entry = new KeyValuePair(entryOrKey as K, value);
    }
    this._table.set(entry.key, entry.value);
    this._publisher.push(entry);
  }

  /**
   * Removes an entry and publishes its previous value if it exists.
   * @param key - The key to remove.
   */
  public erase(key: K): void;

  /**
   * Removes an entry and publishes a specified deletion value if it exists.
   * @param key - The key to remove.
   * @param value - The value to publish indicating deletion.
   */
  public erase(key: K, value: V): void;
  public erase(key: K, value?: V): void {
    if(!this._table.has(key)) {
      return;
    }
    if(arguments.length === 1) {
      value = this._table.get(key);
    }
    this._table.delete(key);
    this._publisher.push(new KeyValuePair(key, value));
  }

  /** Returns a shallow copy of the table, including after closure. */
  public getSnapshot(): Map<K, V> {
    return this._table.clone();
  }

  /**
   * Sends the current table to a queue and subscribes it to later updates.
   * @param queue - The queue receiving the entries and subsequent updates.
   */
  public monitor(queue: QueueWriter<KeyValuePair<K, V>>): void;

  /**
   * Captures the table and subscribes a queue to subsequent updates.
   * @param queue - The queue receiving updates after the captured snapshot.
   * @param snapshot - Receives a shallow copy of the table.
   */
  public monitor(queue: QueueWriter<KeyValuePair<K, V>>,
    snapshot: { value: Map<K, V> }): void;
  public monitor(queue: QueueWriter<KeyValuePair<K, V>>,
      snapshot?: { value: Map<K, V> }): void {
    if(snapshot) {
      snapshot.value = this.getSnapshot();
    } else {
      for(const [key, value] of this._table) {
        queue.push(new KeyValuePair(key, value));
      }
    }
    this._publisher.monitor(queue);
  }

  /**
   * Closes monitored queues and retains the current table.
   * @param error - The error used to close the queues.
   */
  public close(error?: Error): void {
    this._publisher.close(error);
  }

  private _table: Map<K, V>;
  private _publisher: QueueWriterPublisher<KeyValuePair<K, V>>;
}
