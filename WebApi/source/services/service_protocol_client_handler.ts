import { ConnectError } from '../io/connect_error';
import { PipeBrokenError } from '../queues/pipe_broken_error';
import { QueueWriter } from '../queues/queue_writer';
import { Message, MessageType } from './message';
import { ServiceProtocolClient } from './service_protocol_client';

/** The delay between failed connection attempts, in milliseconds. */
const RECONNECT_INTERVAL = 1000;

/** Maintains a service connection and dispatches messages across reconnects. */
export class ServiceProtocolClientHandler {

  /**
   * Constructs a connection handler for a WebSocket endpoint.
   * @param serviceUrl - The service's WebSocket endpoint.
   */
  constructor(serviceUrl: URL);

  /**
   * Constructs a connection handler with query recovery.
   * @param serviceUrl - The service's WebSocket endpoint.
   * @param reconnectHandler - Recovers queries on the opened replacement
   * client.
   * Use this client directly. getClient waits until recovery finishes.
   * A recovery failure stops the handler.
   */
  constructor(serviceUrl: URL,
    reconnectHandler: (client: ServiceProtocolClient) => void | Promise<void>);

  /**
   * Constructs a connection handler with a retry delay.
   * @param serviceUrl - The service's WebSocket endpoint.
   * @param reconnectHandler - Recovers queries on the opened replacement
   * client.
   * @param reconnectInterval - Milliseconds between failed connection attempts.
   */
  constructor(serviceUrl: URL,
    reconnectHandler: (client: ServiceProtocolClient) => void | Promise<void>,
    reconnectInterval: number);

  /**
   * Constructs a connection handler.
   * @param builder - Creates a fresh, unopened client. Connection failures must
   * throw ConnectError. Other failures stop the handler.
   */
  constructor(builder: () => ServiceProtocolClient);

  /**
   * Constructs a connection handler with query recovery.
   * @param builder - Creates a fresh, unopened client.
   * @param reconnectHandler - Recovers queries on a replacement client. Use
   * this client directly. getClient waits until recovery finishes. A failure
   * stops the handler.
   */
  constructor(builder: () => ServiceProtocolClient,
    reconnectHandler: (client: ServiceProtocolClient) => void | Promise<void>);

  /**
   * Constructs a connection handler with a retry delay.
   * @param builder - Creates a fresh, unopened client.
   * @param reconnectHandler - Recovers queries on a replacement client.
   * @param reconnectInterval - Milliseconds between failed connection attempts.
   */
  constructor(builder: () => ServiceProtocolClient,
    reconnectHandler: (client: ServiceProtocolClient) => void | Promise<void>,
    reconnectInterval: number);
  constructor(source: URL | (() => ServiceProtocolClient),
      reconnectHandler?:
        (client: ServiceProtocolClient) => void | Promise<void>,
      reconnectInterval?: number) {
    if(source instanceof URL) {
      this._builder = () => new ServiceProtocolClient(source);
    } else {
      this._builder = source;
    }
    this._reconnectHandler = reconnectHandler ?? (() => {});
    this._reconnectInterval = reconnectInterval ?? RECONNECT_INTERVAL;
    if(!Number.isFinite(this._reconnectInterval) ||
        this._reconnectInterval < 0) {
      throw new RangeError('Invalid reconnect interval.');
    }
    this._client = null;
    this._candidate = null;
    this._connecting = null;
    this._resolveClient = null;
    this._rejectClient = null;
    this._hasConnected = false;
    this._isStarted = false;
    this._exception = null;
    this._monitors = new Map();
    this._timer = null;
    this._wake = null;
    this._closed = new Promise<never>((resolve, reject) => {
      this._rejectClosed = reject;
    });
    this._closed.catch(() => {});
  }

  /**
   * Returns the connected client, starting the connection loop on first use.
   * Concurrent calls share the connection attempt and wait for recovery.
   * @throws Error If the handler is closed or a non-connection failure occurs.
   */
  public getClient(): Promise<ServiceProtocolClient> {
    if(this._exception) {
      return Promise.reject(this._exception);
    }
    if(this._client) {
      return Promise.resolve(this._client);
    }
    if(!this._connecting) {
      this._connecting = new Promise((resolve, reject) => {
        this._resolveClient = resolve;
        this._rejectClient = reject;
      });
    }
    const connecting = this._connecting;
    if(!this._isStarted) {
      this._isStarted = true;
      this.run();
    }
    return connecting;
  }

  /**
   * Registers a message monitor that persists across reconnections.
   * @param messageType - The message type to deserialize.
   * @param queue - Receives messages and closes when this handler stops.
   */
  public monitor<T extends Message>(messageType: MessageType<T>,
      queue: QueueWriter<T>): void {
    if(this._exception) {
      queue.close(this._exception);
      return;
    }
    this._monitors.set(messageType.TYPE, {
      deserialize: messageType.fromJson,
      queue
    });
  }

  /**
   * Stops retries, closes the client and monitors, and rejects waiting callers.
   * @param error - The error reported to waiting callers and monitors.
   */
  public close(error?: Error): void {
    if(this._exception) {
      return;
    }
    this._exception = error ?? new PipeBrokenError();
    this._rejectClosed(this._exception);
    this._rejectClient?.(this._exception);
    this._connecting = null;
    this._resolveClient = null;
    this._rejectClient = null;
    this._client?.close();
    this._candidate?.close();
    this._client = null;
    this._candidate = null;
    if(this._timer !== null) {
      clearTimeout(this._timer);
      this._timer = null;
      this._wake();
      this._wake = null;
    }
    for(const monitor of this._monitors.values()) {
      monitor.queue.close(this._exception);
    }
    this._monitors.clear();
  }

  private async run(): Promise<void> {
    while(!this._exception) {
      let client: ServiceProtocolClient = null;
      try {
        client = this._builder();
        if(this._exception) {
          client.close();
          return;
        }
        this._candidate = client;
        await Promise.race([client.open(), this._closed]);
      } catch(error) {
        client?.close();
        this._candidate = null;
        if(this._exception) {
          return;
        }
        if(error instanceof ConnectError) {
          await this.wait();
          continue;
        }
        this.close(error as Error);
        return;
      }
      if(this._exception) {
        client.close();
        return;
      }
      const reading = this.readMessages(client);
      try {
        if(this._hasConnected) {
          const recovery = Promise.resolve().then(() => {
            if(this._exception) {
              throw this._exception;
            }
            return this._reconnectHandler(client);
          });
          await Promise.race([recovery, reading, this._closed]);
        }
        if(this._exception) {
          return;
        }
        this._client = client;
        this._candidate = null;
        this._hasConnected = true;
        this._resolveClient?.(client);
        this._connecting = null;
        this._resolveClient = null;
        this._rejectClient = null;
      } catch(error) {
        this.close(error as Error);
        return;
      }
      try {
        await reading;
      } catch {
      } finally {
        this._client = null;
        client.close();
      }
    }
  }

  private wait(): Promise<void> {
    return new Promise(resolve => {
      this._wake = resolve;
      this._timer = setTimeout(() => {
        this._timer = null;
        this._wake = null;
        resolve();
      }, this._reconnectInterval);
    });
  }

  private async readMessages(client: ServiceProtocolClient): Promise<never> {
    while(true) {
      const message = await client.readMessage();
      if(this._exception) {
        throw this._exception;
      }
      const monitor = this._monitors.get(message.type);
      if(monitor) {
        monitor.queue.push(monitor.deserialize(message.data));
      }
    }
  }

  private _builder: () => ServiceProtocolClient;
  private _reconnectHandler:
    (client: ServiceProtocolClient) => void | Promise<void>;
  private _reconnectInterval: number;
  private _client: ServiceProtocolClient;
  private _candidate: ServiceProtocolClient;
  private _connecting: Promise<ServiceProtocolClient>;
  private _resolveClient: (client: ServiceProtocolClient) => void;
  private _rejectClient: (error: Error) => void;
  private _hasConnected: boolean;
  private _isStarted: boolean;
  private _exception: Error;
  private _monitors: Map<string, {
    deserialize: (data: any) => Message;
    queue: QueueWriter<Message>;
  }>;
  private _timer: ReturnType<typeof setTimeout>;
  private _wake: () => void;
  private _closed: Promise<never>;
  private _rejectClosed: (error: Error) => void;
}
