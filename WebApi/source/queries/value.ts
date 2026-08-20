import { equals } from '../comparators';
import { QueryType } from './query_type';

/** Base class used to encapsulate a value used in a query. */
export abstract class Value {

  /**
   * Registers a type of Value so that it can be parsed from JSON.
   * @param name - The name identifying the type.
   * @param parser - Constructs a Value from its JSON representation.
   */
  public static register(name: string, parser: Value.Parser): void {
    PARSERS.set(name, parser);
  }

  /** Constructs a Value from a JSON object. */
  public static fromJson(value: any): Value {
    const parser = PARSERS.get(value.__type);
    if(parser === undefined) {
      throw new TypeError(`Unregistered value type: ${value.__type}`);
    }
    return parser(value);
  }

  /** Returns the value's data type. */
  public abstract get type(): QueryType;

  /** Tests if two values store the same data. */
  public abstract equals(other: Value): boolean;

  public abstract toString(): string;

  /** Converts this object to JSON. */
  public abstract toJson(): any;
}

export namespace Value {

  /** Constructs a Value from its JSON representation. */
  export type Parser = (value: any) => Value;
}

/** Stores a Value using a native type. */
export abstract class NativeValue<T> extends Value {

  /** Returns the stored value. */
  public get value(): T {
    return this._value;
  }

  public equals(other: Value): boolean {
    return other instanceof NativeValue && this.type === other.type &&
      equals(this._value, other._value);
  }

  public toString(): string {
    return `${this._value}`;
  }

  public toJson(): any {
    return {
      __type: this.name,
      value: this.valueToJson()
    };
  }

  /**
   * Constructs a NativeValue.
   * @param value - The value to store.
   */
  protected constructor(value: T) {
    super();
    this._value = value;
  }

  /** Returns the name identifying this type of Value. */
  protected abstract get name(): string;

  /** Converts the stored value to JSON. */
  protected valueToJson(): any {
    return this._value;
  }

  private _value: T;
}

const PARSERS = new Map<string, Value.Parser>();
