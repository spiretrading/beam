/** Indicates a service request failed. */
export class ServiceError {

  /** Parses a ServiceError from a JSON object. */
  public static fromJson(value: any): ServiceError {
    return new ServiceError(value.message, value.code);
  }

  /** Constructs a ServiceError.
   * @param message - The reason for the failure.
   * @param code - An optional error code.
   */
  public constructor(message: string, code: number = 0) {
    this._message = message;
    this._code = code;
  }

  /** Returns the error message. */
  public get message(): string {
    return this._message;
  }

  /** Returns the error code. */
  public get code(): number {
    return this._code;
  }

  public toString(): string {
    return this._message;
  }

  private _message: string;
  private _code: number;
}
