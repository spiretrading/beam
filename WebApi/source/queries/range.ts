import { Date, DateTime } from '../definitions';
import { Sequence } from './sequence';

/**
 * Defines the range of values to query over in terms of time or Sequence
 * points.
 */
export class Range {

  /** Represents an empty Range. */
  public static readonly EMPTY = new Range(Sequence.FIRST, Sequence.FIRST);

  /** Represents all historical sequences. */
  public static readonly HISTORICAL =
    new Range(Sequence.FIRST, Sequence.PRESENT);

  /** Represents the entire sequence including real time data. */
  public static readonly TOTAL = new Range(Sequence.FIRST, Sequence.LAST);

  /** Represents real time data. */
  public static readonly REAL_TIME = new Range(Sequence.PRESENT, Sequence.LAST);

  /** Constructs a Range from a JSON object. */
  public static fromJson(value: any): Range {
    return new Range(pointFromJson(value.start), pointFromJson(value.end));
  }

  /**
   * Constructs a Range.
   * @param start - The start of the Range.
   * @param end - The end of the Range.
   */
  constructor(
      start: Range.Point = Sequence.FIRST, end: Range.Point = Sequence.FIRST) {
    if(!isValid(start) || !isValid(end)) {
      this._start = Sequence.FIRST;
      this._end = Sequence.FIRST;
      return;
    }
    this._start = normalize(start);
    this._end = normalize(end);
  }

  /** Returns the start of the Range. */
  public get start(): Range.Point {
    return this._start;
  }

  /** Returns the end of the Range. */
  public get end(): Range.Point {
    return this._end;
  }

  /** Tests if two ranges span the same points. */
  public equals(other: Range): boolean {
    return other && pointEquals(this._start, other._start) &&
      pointEquals(this._end, other._end);
  }

  public toString(): string {
    if(this.equals(Range.EMPTY)) {
      return 'Empty';
    } else if(this.equals(Range.TOTAL)) {
      return 'Total';
    }
    return `(${this._start} ${this._end})`;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      start: pointToJson(this._start),
      end: pointToJson(this._end)
    };
  }

  private _start: Range.Point;
  private _end: Range.Point;
}

export namespace Range {

  /** Stores a single end-point in a range as either a time or a Sequence. */
  export type Point = Sequence | DateTime;
}

function isValid(point: Range.Point): boolean {
  if(point instanceof DateTime) {
    return !point.date.equals(Date.NOT_A_DATE);
  }
  return true;
}

function normalize(point: Range.Point): Range.Point {
  if(point instanceof DateTime) {
    if(point.date.equals(Date.NEG_INFIN)) {
      return Sequence.FIRST;
    } else if(point.date.equals(Date.POS_INFIN)) {
      return Sequence.LAST;
    }
  }
  return point;
}

function pointEquals(left: Range.Point, right: Range.Point): boolean {
  if(left instanceof Sequence) {
    return right instanceof Sequence && left.equals(right);
  }
  return right instanceof DateTime && left.equals(right);
}

function pointFromJson(value: any): Range.Point {
  if(value.which === 0) {
    return Sequence.fromJson(value.value);
  }
  return DateTime.fromJson(value.value);
}

function pointToJson(point: Range.Point): any {
  if(point instanceof Sequence) {
    return {
      which: 0,
      value: point.toJson()
    };
  }
  return {
    which: 1,
    value: point.toJson()
  };
}
