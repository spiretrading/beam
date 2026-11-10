/** Indicates that a connection could not be established. */
export class ConnectError extends Error {

  /** Constructs a ConnectError. */
  constructor();

  /**
   * Constructs a ConnectError with a message.
   * @param message - The reason the connection failed.
   */
  constructor(message: string);
  constructor(message?: string) {
    super(message ?? 'Unable to connect.');
  }
}
