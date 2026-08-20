/**
 * The type that a query value or expression evaluates to, named as it is
 * registered by the service it is sent to.
 */
export type QueryType = string;

export namespace QueryType {

  /** A boolean value. */
  export const BOOL: QueryType = 'bool';

  /** A single character. */
  export const CHAR: QueryType = 'char';

  /** A signed 32-bit integer. */
  export const INT: QueryType = 'int';

  /** A double precision floating point value. */
  export const DECIMAL: QueryType = 'double';

  /** An unsigned 64-bit identifier. */
  export const ID: QueryType = 'uint64';

  /** A sequence of characters. */
  export const STRING: QueryType = 'string';

  /** A point in time. */
  export const DATE_TIME: QueryType = 'boost.posix_time.ptime';

  /** A length of time. */
  export const DURATION: QueryType = 'boost.posix_time.time_duration';
}
