import { equals } from '../comparators';
import { fromJson, toJson } from '../serialization';

/** Stores a value and its index. */
export class IndexedValue<V, I> {

  /** Constructs an IndexedValue from a JSON object.
   * @param valueType - The type of value to parse.
   * @param indexType - The type of index to parse.
   * @param value - The JSON object to parse.
   */
  public static fromJson<V, I>(valueType: any, indexType: any,
      value: any): IndexedValue<V, I> {
    return new IndexedValue<V, I>(
      fromJson(valueType, value.value), fromJson(indexType, value.index));
  }

  /**
   * Constructs an IndexedValue.
   * @param value - The value to store.
   * @param index - The value's index.
   */
  constructor(value: V, index: I) {
    this._value = value;
    this._index = index;
  }

  /** Returns the value. */
  public get value(): V {
    return this._value;
  }

  /** Returns the index. */
  public get index(): I {
    return this._index;
  }

  /** Tests if two indexed values are equal. */
  public equals(other: IndexedValue<V, I>): boolean {
    return other && equals(this._value, other._value) &&
      equals(this._index, other._index);
  }

  public toString(): string {
    return `(${this._index} ${this._value})`;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      value: toJson(this._value),
      index: toJson(this._index)
    };
  }

  private _value: V;
  private _index: I;
}
