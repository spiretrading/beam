import { equals } from '../comparators';
import { fromJson, toJson } from '../serialization';
import { ConstantExpression } from './constant_expression';
import { Expression } from './expression';
import { checkFilter } from './filtered_query';
import { SnapshotLimit } from './snapshot_limit';

/**
 * Composes various queries into a query whose results are returned in pages.
 * To use it, specify an optional anchor point around which results are
 * returned. For example to query for 20 city names that come after 'Toronto',
 * set the anchor to 'Toronto' and the snapshot limit to head 20. The anchor
 * can be left empty to query the first or last value in a data set.
 */
export class PagedQuery<I, A> {

  /** Constructs a PagedQuery from a JSON object.
   * @param indexType - The type of index to parse.
   * @param anchorType - The type of anchor to parse.
   * @param value - The JSON object to parse.
   */
  public static fromJson<I, A>(
      indexType: any, anchorType: any, value: any): PagedQuery<I, A> {
    const query = new PagedQuery<I, A>(fromJson(indexType, value.index));
    query.snapshotLimit = SnapshotLimit.fromJson(value.snapshot_limit);
    query.filter = Expression.nestedFromJson(value.filter);
    if(value.anchor && value.anchor.is_initialized) {
      query.anchor = fromJson(anchorType, value.anchor.value);
    }
    query.offset = value.offset;
    return query;
  }

  /**
   * Constructs a PagedQuery with no anchor and no offset.
   * @param index - The index to query.
   */
  constructor(index: I) {
    this._index = index;
    this._snapshotLimit = SnapshotLimit.NONE;
    this._filter = ConstantExpression.TRUE;
    this._anchor = null as A;
    this._offset = 0;
  }

  /** Returns the index. */
  public get index(): I {
    return this._index;
  }

  public set index(value: I) {
    this._index = value;
  }

  /** Returns the SnapshotLimit. */
  public get snapshotLimit(): SnapshotLimit {
    return this._snapshotLimit;
  }

  public set snapshotLimit(value: SnapshotLimit) {
    this._snapshotLimit = value;
  }

  /** Returns the filter applied to the values returned. */
  public get filter(): Expression {
    return this._filter;
  }

  public set filter(value: Expression) {
    this._filter = checkFilter(value);
  }

  /** Returns the anchor, or null if the query has no anchor. */
  public get anchor(): A {
    return this._anchor;
  }

  public set anchor(value: A) {
    this._anchor = value;
  }

  /** Returns the number of values to skip from the anchor. */
  public get offset(): number {
    return this._offset;
  }

  public set offset(value: number) {
    this._offset = Math.max(0, value);
  }

  /** Tests if two queries are equivalent. */
  public equals(other: PagedQuery<I, A>): boolean {
    return other && equals(this._index, other._index) &&
      this._snapshotLimit.equals(other._snapshotLimit) &&
      this._filter.equals(other._filter) &&
      anchorEquals(this._anchor, other._anchor) &&
      this._offset === other._offset;
  }

  public toString(): string {
    const anchor = (() => {
      if(this._anchor === null || this._anchor === undefined) {
        return '';
      }
      return `${this._anchor} `;
    })();
    const offset = (() => {
      if(this._offset === 0) {
        return '';
      }
      return `${this._offset} `;
    })();
    return `(${this._index} ${this._snapshotLimit} ${anchor}${offset}` +
      `${this._filter})`;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      index: toJson(this._index),
      snapshot_limit: this._snapshotLimit.toJson(),
      filter: Expression.nestedToJson(this._filter),
      anchor: anchorToJson(this._anchor),
      offset: this._offset
    };
  }

  private _index: I;
  private _snapshotLimit: SnapshotLimit;
  private _filter: Expression;
  private _anchor: A;
  private _offset: number;
}

function anchorEquals(left: any, right: any): boolean {
  if(left === null || left === undefined) {
    return right === null || right === undefined;
  } else if(right === null || right === undefined) {
    return false;
  }
  return equals(left, right);
}

function anchorToJson(anchor: any): any {
  if(anchor === null || anchor === undefined) {
    return {
      is_initialized: false
    };
  }
  return {
    is_initialized: true,
    value: toJson(anchor)
  };
}
