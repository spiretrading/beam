import { Message } from './message';

/** A keepalive message sent at fixed intervals. */
export class HeartbeatMessage extends Message {
  public static readonly TYPE = 'Beam.Services.HeartbeatMessage';

  public static fromJson(): HeartbeatMessage {
    return new HeartbeatMessage();
  }

  constructor() {
    super(HeartbeatMessage.TYPE, {});
  }
}
