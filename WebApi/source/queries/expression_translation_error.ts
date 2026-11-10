/** Signals a failure to translate an expression. */
export class ExpressionTranslationError extends Error {

  /**
   * Constructs an ExpressionTranslationError with a message.
   * @param message - The description of the error.
   */
  constructor(message: string) {
    super(message);
  }
}
