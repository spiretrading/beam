import { fromJson, toJson } from '../serialization';
import { makeAllValuesFilter } from './filter';
import { InterruptionPolicy } from './interruption_policy';
import { Range } from './range';
import { SnapshotLimit } from './snapshot_limit';

/**
 * Composes the standard query types into a query with common and basic
 * functionality.
 */
export class BasicQuery<T> {

  /** Constructs a BasicQuery from a JSON object.
   * @param type - The type of index to parse.
   * @param value - The JSON object to parse.
   */
  public static fromJson<T>(type: any, value: any): BasicQuery<T> {
    const query = new BasicQuery<T>(fromJson(type, value.index));
    query.range = Range.fromJson(value.range);
    query.snapshotLimit = SnapshotLimit.fromJson(value.snapshot_limit);
    query.interruptionPolicy = value.interruption_policy;
    return query;
  }

  /**
   * Returns a BasicQuery that streams the most recent value produced.
   * @param index - The index to query.
   */
  public static makeCurrentQuery<T>(index: T): BasicQuery<T> {
    const query = new BasicQuery<T>(index);
    query.range = Range.TOTAL;
    query.snapshotLimit = SnapshotLimit.fromTail(1);
    query.interruptionPolicy = InterruptionPolicy.IGNORE_CONTINUE;
    return query;
  }

  /**
   * Returns a BasicQuery that retrieves only the latest value and then breaks.
   * @param index - The index to query.
   */
  public static makeLatestQuery<T>(index: T): BasicQuery<T> {
    const query = new BasicQuery<T>(index);
    query.range = Range.HISTORICAL;
    query.snapshotLimit = SnapshotLimit.fromTail(1);
    return query;
  }

  /**
   * Returns a BasicQuery that streams real time values.
   * @param index - The index to query.
   */
  public static makeRealTimeQuery<T>(index: T): BasicQuery<T> {
    const query = new BasicQuery<T>(index);
    query.range = Range.REAL_TIME;
    query.interruptionPolicy = InterruptionPolicy.IGNORE_CONTINUE;
    return query;
  }

  /**
   * Constructs a BasicQuery.
   * @param index - The index to query.
   */
  constructor(index: T) {
    this._index = index;
    this._range = Range.EMPTY;
    this._snapshotLimit = SnapshotLimit.NONE;
    this._interruptionPolicy = InterruptionPolicy.BREAK_QUERY;
  }

  /** Returns the index. */
  public get index(): T {
    return this._index;
  }

  public set index(value: T) {
    this._index = value;
  }

  /** Returns the Range to query. */
  public get range(): Range {
    return this._range;
  }

  public set range(value: Range) {
    this._range = value;
  }

  /** Returns the SnapshotLimit. */
  public get snapshotLimit(): SnapshotLimit {
    return this._snapshotLimit;
  }

  public set snapshotLimit(value: SnapshotLimit) {
    this._snapshotLimit = value;
  }

  /** Returns the InterruptionPolicy. */
  public get interruptionPolicy(): InterruptionPolicy {
    return this._interruptionPolicy;
  }

  public set interruptionPolicy(value: InterruptionPolicy) {
    this._interruptionPolicy = value;
  }

  public toString(): string {
    return `(${this._index} ${this._range} ${this._snapshotLimit} ` +
      `${InterruptionPolicy[this._interruptionPolicy]} true)`;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      index: toJson(this._index),
      range: this._range.toJson(),
      snapshot_limit: this._snapshotLimit.toJson(),
      interruption_policy: this._interruptionPolicy,
      filter: makeAllValuesFilter()
    };
  }

  private _index: T;
  private _range: Range;
  private _snapshotLimit: SnapshotLimit;
  private _interruptionPolicy: InterruptionPolicy;
}
