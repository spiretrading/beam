/** A type that can deserialize a Message from JSON. */
export interface MessageType<T extends Message> {

  /** Deserializes a Message from its data payload. */
  fromJson(data: any): T;
}

/** Defines a one-way message sent or received over a service connection. */
export class Message {

  /**
   * Deserializes a Message from JSON.
   * @param type - The Message subclass to deserialize into.
   * @param value - The JSON object to deserialize.
   * @returns The deserialized Message.
   */
  public static fromJson<T extends Message>(
      type: MessageType<T>, value: any): T {
    const { __type, __version, ...data } = value;
    return type.fromJson(data);
  }

  /**
   * Constructs a Message.
   * @param type - The message type name.
   * @param version - The message version.
   * @param data - The JSON payload.
   */
  protected constructor(public readonly type: string, public readonly data: any,
    public readonly version: number = 0) {}

  /** Serializes the message to JSON. */
  public toJson(): any {
    return {
      __type: this.type,
      __version: this.version,
      ...this.data
    };
  }
}
