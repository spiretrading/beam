import { equals } from '../comparators';
import { arrayFromJson, toJson } from '../serialization';

/** Stores the result of a query. */
export class QueryResult<T> {

  /** Constructs a QueryResult from a JSON object.
   * @param type - The type of data returned by the query.
   * @param value - The JSON object to parse.
   */
  public static fromJson<T>(type: any, value: any): QueryResult<T> {
    return new QueryResult<T>(value.id, arrayFromJson(type, value.snapshot));
  }

  /**
   * Constructs a QueryResult.
   * @param id - The query's unique id.
   * @param snapshot - A snapshot of available data from the query.
   */
  constructor(id: number = -1, snapshot: T[] = []) {
    this._id = id;
    this._snapshot = snapshot;
  }

  /** Returns the query's unique id. */
  public get id(): number {
    return this._id;
  }

  /** Returns a snapshot of available data from the query. */
  public get snapshot(): T[] {
    return this._snapshot;
  }

  /** Tests if two query results are equal. */
  public equals(other: QueryResult<T>): boolean {
    if(!other || this._id !== other._id ||
        this._snapshot.length !== other._snapshot.length) {
      return false;
    }
    for(let i = 0; i < this._snapshot.length; ++i) {
      if(!equals(this._snapshot[i], other._snapshot[i])) {
        return false;
      }
    }
    return true;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      id: this._id,
      snapshot: this._snapshot.map(toJson)
    };
  }

  private _id: number;
  private _snapshot: T[];
}
