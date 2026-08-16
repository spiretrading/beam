import { equals } from '../comparators';
import { fromJson, toJson } from '../serialization';

/** Queries over a value used as an index. */
export class IndexedQuery<T> {

  /** Constructs an IndexedQuery from a JSON object.
   * @param type - The type of index to parse.
   * @param value - The JSON object to parse.
   */
  public static fromJson<T>(type: any, value: any): IndexedQuery<T> {
    return new IndexedQuery<T>(fromJson(type, value.index));
  }

  /**
   * Constructs an IndexedQuery.
   * @param index - The index to query.
   */
  constructor(index: T) {
    this._index = index;
  }

  /** Returns the index. */
  public get index(): T {
    return this._index;
  }

  public set index(value: T) {
    this._index = value;
  }

  /** Tests if two queries specify the same index. */
  public equals(other: IndexedQuery<T>): boolean {
    return other && equals(this._index, other._index);
  }

  public toString(): string {
    return `${this._index}`;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      index: toJson(this._index)
    };
  }

  private _index: T;
}
