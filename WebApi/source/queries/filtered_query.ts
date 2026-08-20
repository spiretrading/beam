import { ConstantExpression } from './constant_expression';
import { Expression } from './expression';
import { QueryType } from './query_type';

/** Filters what values should be returned in a query. */
export class FilteredQuery {

  /** Constructs a FilteredQuery from a JSON object. */
  public static fromJson(value: any): FilteredQuery {
    return new FilteredQuery(Expression.nestedFromJson(value.filter));
  }

  /**
   * Constructs a FilteredQuery.
   * @param filter - The Expression used as the filter.
   */
  constructor(filter: Expression = ConstantExpression.TRUE) {
    this._filter = checkFilter(filter);
  }

  /** Returns the filter. */
  public get filter(): Expression {
    return this._filter;
  }

  public set filter(value: Expression) {
    this._filter = checkFilter(value);
  }

  /** Tests if two queries specify the same filter. */
  public equals(other: FilteredQuery): boolean {
    return other && this._filter.equals(other._filter);
  }

  public toString(): string {
    return this._filter.toString();
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      filter: Expression.nestedToJson(this._filter)
    };
  }

  private _filter: Expression;
}

/**
 * Tests that an Expression can be used as a filter.
 * @param filter - The Expression to test.
 * @return The filter that was tested.
 */
export function checkFilter(filter: Expression): Expression {
  if(filter.type !== QueryType.BOOL) {
    throw new TypeError('Filter is not boolean.');
  }
  return filter;
}
