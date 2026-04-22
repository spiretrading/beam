/**
 * Defines a service request with a typed response.
 * @param R - The type of the response.
 */
export interface ServiceRequest<R> {

  /** The service name used in the wire protocol. */
  readonly service: string;

  /** Serializes the request parameters to JSON. */
  toJson(): any;

  /** Deserializes the response from JSON. */
  parseResponse(value: any): R;
}
