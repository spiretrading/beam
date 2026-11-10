/** Signals that a query was interrupted. */
export class QueryInterruptedError extends Error {

  /** Constructs a QueryInterruptedError. */
  constructor();

  /**
   * Constructs a QueryInterruptedError with a message.
   * @param message - The description of the error.
   */
  constructor(message: string);
  constructor(message?: string) {
    super(message ?? 'Query interrupted.');
  }
}
