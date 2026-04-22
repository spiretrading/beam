import { Message } from './message';

/** A Message representing a service request. */
export class RequestMessage extends Message {

  /**
   * Constructs a RequestMessage.
   * @param type - The service type name.
   * @param id - The unique request identifier.
   * @param parameters - The request parameters.
   */
  constructor(type: string, public readonly id: number, parameters: any) {
    super(type + '.Request', Object.keys(parameters).length > 0 ?
      { request_id: id, parameters } : { request_id: id });
  }
}
