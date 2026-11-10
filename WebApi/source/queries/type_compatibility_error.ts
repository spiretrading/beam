/** Signals incompatible query value types. */
export class TypeCompatibilityError extends Error {

  /** Constructs a TypeCompatibilityError. */
  constructor();

  /**
   * Constructs a TypeCompatibilityError with a message.
   * @param message - The description of the error.
   */
  constructor(message: string);
  constructor(message?: string) {
    super(message ?? 'Incompatible types.');
  }
}
