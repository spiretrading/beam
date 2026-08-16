import { equals } from '../comparators';
import { fromJson, toJson } from '../serialization';
import { Sequence } from './sequence';

/** Stores a value that is part of a Sequence. */
export class SequencedValue<T> {

  /** Constructs a SequencedValue from a JSON object.
   * @param type - The type of value to parse.
   * @param value - The JSON object to parse.
   */
  public static fromJson<T>(type: any, value: any): SequencedValue<T> {
    return new SequencedValue<T>(
      fromJson(type, value.value), Sequence.fromJson(value.sequence));
  }

  /**
   * Constructs a SequencedValue.
   * @param value - The value to store.
   * @param sequence - The value's Sequence.
   */
  constructor(value: T, sequence: Sequence = Sequence.FIRST) {
    this._value = value;
    this._sequence = sequence;
  }

  /** Returns the value. */
  public get value(): T {
    return this._value;
  }

  /** Returns the Sequence. */
  public get sequence(): Sequence {
    return this._sequence;
  }

  /** Tests if two sequenced values are equal. */
  public equals(other: SequencedValue<T>): boolean {
    return other && equals(this._value, other._value) &&
      this._sequence.equals(other._sequence);
  }

  public toString(): string {
    return `(${this._value} ${this._sequence})`;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      value: toJson(this._value),
      sequence: this._sequence.toJson()
    };
  }

  private _value: T;
  private _sequence: Sequence;
}
