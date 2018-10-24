/** Indicates a service request failed. */
export class ServiceError {

  /** Constructs a ServiceError.
   * @param message - The reason for the failure.
   */
  public constructor(message: string) {
    this._message = message;
  }

  /** Returns the error message. */
  public get message(): string {
    return this._message;
  }

  public toString(): string {
    return this._message;
  }

  private _message: string;
}
