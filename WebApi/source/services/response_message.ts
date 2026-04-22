import { Message } from './message';

/** A Message representing a service response. */
export class ResponseMessage extends Message {

  /**
   * Constructs a ResponseMessage.
   * @param type - The service type name.
   * @param id - The request identifier this response is for.
   * @param result - The result payload, or undefined if empty.
   * @param isException - Whether the response represents an exception.
   */
  constructor(type: string, public readonly id: number,
      public readonly result?: any,
      public readonly isException: boolean = false) {
    super(type + '.Response', {
      request_id: id,
      is_exception: isException,
      ...(result !== undefined ? { result } : {})
    });
  }
}
