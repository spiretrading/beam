/** Enumerates the ways to recover from a query being interrupted. */
export enum InterruptionPolicy {

  /** Breaks the query. */
  BREAK_QUERY = 0,

  /** Recovers all lost data. */
  RECOVER_DATA,

  /** Ignores all lost data. */
  IGNORE_CONTINUE
}
