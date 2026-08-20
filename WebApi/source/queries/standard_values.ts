import { DateTime, Duration } from '../definitions';
import { QueryType } from './query_type';
import { NativeValue, Value } from './value';

/** Stores a boolean. */
export class BoolValue extends NativeValue<boolean> {

  /** Constructs a BoolValue from a JSON object. */
  public static fromJson(value: any): BoolValue {
    return new BoolValue(value.value);
  }

  /**
   * Constructs a BoolValue.
   * @param value - The value to store.
   */
  constructor(value: boolean) {
    super(value);
  }

  public get type(): QueryType {
    return QueryType.BOOL;
  }

  protected get name(): string {
    return BOOL_NAME;
  }
}

/** Stores a single character. */
export class CharValue extends NativeValue<string> {

  /** Constructs a CharValue from a JSON object. */
  public static fromJson(value: any): CharValue {
    if(typeof value.value === 'number') {
      return new CharValue('\0');
    }
    return new CharValue(value.value);
  }

  /**
   * Constructs a CharValue.
   * @param value - The value to store.
   */
  constructor(value: string) {
    super(value);
  }

  public get type(): QueryType {
    return QueryType.CHAR;
  }

  protected get name(): string {
    return CHAR_NAME;
  }

  protected valueToJson(): any {
    if(this.value === '\0') {
      return 0;
    }
    return this.value;
  }
}

/** Stores a signed 32-bit integer. */
export class IntValue extends NativeValue<number> {

  /** Constructs an IntValue from a JSON object. */
  public static fromJson(value: any): IntValue {
    return new IntValue(value.value);
  }

  /**
   * Constructs an IntValue.
   * @param value - The value to store.
   */
  constructor(value: number) {
    super(value);
  }

  public get type(): QueryType {
    return QueryType.INT;
  }

  protected get name(): string {
    return INT_NAME;
  }
}

/** Stores a double precision floating point value. */
export class DecimalValue extends NativeValue<number> {

  /** Constructs a DecimalValue from a JSON object. */
  public static fromJson(value: any): DecimalValue {
    return new DecimalValue(value.value);
  }

  /**
   * Constructs a DecimalValue.
   * @param value - The value to store.
   */
  constructor(value: number) {
    super(value);
  }

  public get type(): QueryType {
    return QueryType.DECIMAL;
  }

  protected get name(): string {
    return DECIMAL_NAME;
  }
}

/** Stores an unsigned 64-bit identifier. */
export class IdValue extends NativeValue<bigint> {

  /** Constructs an IdValue from a JSON object. */
  public static fromJson(value: any): IdValue {
    return new IdValue(BigInt(value.value));
  }

  /**
   * Constructs an IdValue.
   * @param value - The value to store.
   */
  constructor(value: bigint) {
    super(value);
  }

  public get type(): QueryType {
    return QueryType.ID;
  }

  protected get name(): string {
    return ID_NAME;
  }

  protected valueToJson(): any {
    return this.value.toString();
  }
}

/** Stores a sequence of characters. */
export class StringValue extends NativeValue<string> {

  /** Constructs a StringValue from a JSON object. */
  public static fromJson(value: any): StringValue {
    return new StringValue(value.value);
  }

  /**
   * Constructs a StringValue.
   * @param value - The value to store.
   */
  constructor(value: string) {
    super(value);
  }

  public get type(): QueryType {
    return QueryType.STRING;
  }

  public toString(): string {
    return `"${this.value}"`;
  }

  protected get name(): string {
    return STRING_NAME;
  }
}

/** Stores a point in time. */
export class DateTimeValue extends NativeValue<DateTime> {

  /** Constructs a DateTimeValue from a JSON object. */
  public static fromJson(value: any): DateTimeValue {
    return new DateTimeValue(DateTime.fromJson(value.value));
  }

  /**
   * Constructs a DateTimeValue.
   * @param value - The value to store.
   */
  constructor(value: DateTime) {
    super(value);
  }

  public get type(): QueryType {
    return QueryType.DATE_TIME;
  }

  protected get name(): string {
    return DATE_TIME_NAME;
  }

  protected valueToJson(): any {
    return this.value.toJson();
  }
}

/** Stores a length of time. */
export class DurationValue extends NativeValue<Duration> {

  /** Constructs a DurationValue from a JSON object. */
  public static fromJson(value: any): DurationValue {
    return new DurationValue(Duration.fromJson(value.value));
  }

  /**
   * Constructs a DurationValue.
   * @param value - The value to store.
   */
  constructor(value: Duration) {
    super(value);
  }

  public get type(): QueryType {
    return QueryType.DURATION;
  }

  protected get name(): string {
    return DURATION_NAME;
  }

  protected valueToJson(): any {
    return this.value.toJson();
  }
}

const BOOL_NAME = 'Beam.Queries.BoolValue';
const CHAR_NAME = 'Beam.Queries.CharValue';
const INT_NAME = 'Beam.Queries.IntValue';
const DECIMAL_NAME = 'Beam.Queries.DecimalValue';
const ID_NAME = 'Beam.Queries.IdValue';
const STRING_NAME = 'Beam.Queries.StringValue';
const DATE_TIME_NAME = 'Beam.Queries.DateTimeValue';
const DURATION_NAME = 'Beam.Queries.DurationValue';

Value.register(BOOL_NAME, BoolValue.fromJson);
Value.register(CHAR_NAME, CharValue.fromJson);
Value.register(INT_NAME, IntValue.fromJson);
Value.register(DECIMAL_NAME, DecimalValue.fromJson);
Value.register(ID_NAME, IdValue.fromJson);
Value.register(STRING_NAME, StringValue.fromJson);
Value.register(DATE_TIME_NAME, DateTimeValue.fromJson);
Value.register(DURATION_NAME, DurationValue.fromJson);
