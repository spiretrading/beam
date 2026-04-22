/** Describes a Message subclass with its type name and deserializer. */
export interface MessageType<T extends Message> {

  /** The message type name. */
  readonly TYPE: string;

  /** Deserializes a Message from its data payload. */
  fromJson(data: any): T;
}

/** Defines a one-way message sent or received over a service connection. */
export class Message {

  /**
   * Parses a raw JSON object into a Message.
   * @param value - The raw JSON object.
   * @returns A Message with the type, version, and data extracted.
   */
  public static parse(value: any): Message {
    const { __type, __version, ...data } = value;
    return new Message(__type, data, __version);
  }

  /**
   * Deserializes a Message from JSON.
   * @param type - The Message subclass to deserialize into.
   * @param value - The JSON object to deserialize.
   * @returns The deserialized Message.
   */
  public static deserialize<T extends Message>(
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
