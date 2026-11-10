import { DateTime } from '../definitions/date_time';
import { Duration } from '../definitions/duration';
import { QueryType } from './query_type';

/**
 * Tests whether a native value represents the specified query type.
 * @param type - The expected query type.
 * @param value - The native value to test.
 */
export function matchesQueryType(type: QueryType, value: unknown): boolean {
  switch(type) {
    case QueryType.BOOL:
      return typeof value === 'boolean';
    case QueryType.CHAR:
      return typeof value === 'string' && value.length === 1 &&
        value.charCodeAt(0) <= 127;
    case QueryType.STRING:
      return typeof value === 'string';
    case QueryType.INT:
      return typeof value === 'number' && Number.isInteger(value) &&
        value >= -2147483648 && value <= 2147483647;
    case QueryType.DECIMAL:
      return typeof value === 'number';
    case QueryType.ID:
      return typeof value === 'bigint' && value >= 0n &&
        value <= 18446744073709551615n;
    case QueryType.DATE_TIME:
      return value instanceof DateTime;
    case QueryType.DURATION:
      return value instanceof Duration;
    default:
      return false;
  }
}
