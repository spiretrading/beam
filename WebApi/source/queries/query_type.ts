/** Lists the types that a query expression can evaluate to. */
export enum QueryType {

  /** A boolean value. */
  BOOL = 'bool',

  /** A single character. */
  CHAR = 'char',

  /** A signed 32-bit integer. */
  INT = 'int',

  /** A double precision floating point value. */
  DECIMAL = 'double',

  /** An unsigned 64-bit identifier. */
  ID = 'uint64',

  /** A sequence of characters. */
  STRING = 'string',

  /** A point in time. */
  DATE_TIME = 'boost.posix_time.ptime',

  /** A length of time. */
  DURATION = 'boost.posix_time.time_duration'
}
