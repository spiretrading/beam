import { Range } from './range';

/** Queries for data over a specified Range. */
export class RangedQuery {

  /** Constructs a RangedQuery from a JSON object. */
  public static fromJson(value: any): RangedQuery {
    return new RangedQuery(Range.fromJson(value.range));
  }

  /**
   * Constructs a RangedQuery.
   * @param range - The Range to query over.
   */
  constructor(range: Range = Range.EMPTY) {
    this._range = range;
  }

  /** Returns the Range to query. */
  public get range(): Range {
    return this._range;
  }

  public set range(value: Range) {
    this._range = value;
  }

  /** Tests if two queries span the same Range. */
  public equals(other: RangedQuery): boolean {
    return other && this._range.equals(other._range);
  }

  public toString(): string {
    return this._range.toString();
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      range: this._range.toJson()
    };
  }

  private _range: Range;
}
