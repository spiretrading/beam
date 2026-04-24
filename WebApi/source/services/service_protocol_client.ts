import { PipeBrokenError } from '../queues/pipe_broken_error';
import { Queue } from '../queues/queue';
import { QueueWriter } from '../queues/queue_writer';
import { HeartbeatMessage } from './heartbeat_message';
import { Message, MessageType } from './message';
import { RequestMessage } from './request_message';
import { ResponseMessage } from './response_message';
import { ServiceError } from './service_error';
import { ServiceRequest } from './service_request';

/** The default heartbeat interval in milliseconds. */
const HEARTBEAT_INTERVAL_MS = 30000;

/** Implements a client using the Beam service protocol over WebSocket. */
export class ServiceProtocolClient {

  /**
   * Constructs a ServiceProtocolClient.
   * @param url - The WebSocket endpoint URL.
   * @param heartbeatInterval - The heartbeat interval in milliseconds.
   */
  public constructor(
      url: URL, heartbeatInterval: number = HEARTBEAT_INTERVAL_MS) {
    this._url = url;
    this._heartbeatInterval = heartbeatInterval;
    this._socket = null;
    this._heartbeatTimer = null;
    this._nextId = 1;
    this._pendingRequests = new Map();
    this._monitors = new Map();
    this._messages = new Queue<Message>();
  }

  /** Opens the WebSocket connection. */
  public open(): Promise<void> {
    return new Promise((resolve, reject) => {
      this._socket = new WebSocket(this._url.toString());
      this._socket.binaryType = 'arraybuffer';
      this._socket.onopen = () => {
        this.startHeartbeat();
        resolve();
      };
      this._socket.onerror = () => {
        reject(new ServiceError('Failed to connect.'));
      };
      this._socket.onclose = () => {
        this.onClose();
      };
      this._socket.onmessage = (event) => {
        const buffer = event.data as ArrayBuffer;
        this.onMessage(new TextDecoder().decode(buffer.slice(4)));
      };
    });
  }

  /**
   * Closes the connection, rejects pending requests, closes all monitors,
   * and closes the message queue.
   */
  public close(): void {
    if(this._socket) {
      this._socket.onclose = null;
      this._socket.close();
      this._socket = null;
    }
    this.onClose();
  }

  /**
   * Sends a typed request and waits for the response.
   * @param request - The service request.
   * @returns The deserialized response.
   */
  public async sendRequest<R>(request: ServiceRequest<R>): Promise<R> {
    const id = this._nextId;
    ++this._nextId;
    const message = new RequestMessage(request.service, id, request.toJson());
    const response = new Promise<ResponseMessage>((resolve, reject) => {
      this._pendingRequests.set(id, { resolve, reject });
    });
    this.send(message);
    const result = await response;
    if(result.isException) {
      throw new ServiceError(result.result);
    }
    return request.parseResponse(result.result);
  }

  /**
   * Sends a one-way message.
   * @param message - The message to send.
   */
  public sendMessage(message: Message): void {
    this.send(message);
  }

  /**
   * Registers a queue to receive pushed messages of a specific type.
   * @param messageType - The Message subclass to monitor.
   * @param queue - The queue to push deserialized messages onto.
   */
  public monitor<T extends Message>(
      messageType: MessageType<T>, queue: QueueWriter<T>): void {
    this._monitors.set(messageType.TYPE, {
      deserialize: messageType.fromJson,
      queue
    });
  }

  /**
   * Returns the next unhandled pushed message, waiting until one is
   * available.
   * @returns The received message.
   */
  public readMessage(): Promise<Message> {
    return this._messages.pop();
  }

  private send(message: Message): void {
    const encoded = new TextEncoder().encode(JSON.stringify(message.toJson()));
    const buffer = new ArrayBuffer(4 + encoded.byteLength);
    new DataView(buffer).setUint32(0, encoded.byteLength, true);
    new Uint8Array(buffer, 4).set(encoded);
    this._socket.send(buffer);
  }

  private startHeartbeat(): void {
    this._heartbeatTimer = setInterval(() => {
      this.sendMessage(new HeartbeatMessage());
    }, this._heartbeatInterval);
  }

  private stopHeartbeat(): void {
    if(this._heartbeatTimer !== null) {
      clearInterval(this._heartbeatTimer);
      this._heartbeatTimer = null;
    }
  }

  private onMessage(data: string): void {
    const json = JSON.parse(data);
    const { __type, __version, ...payload } = json;
    if(__type === HeartbeatMessage.TYPE) {
      return;
    }
    if(payload.request_id !== undefined && payload.is_exception !== undefined) {
      const pending = this._pendingRequests.get(payload.request_id);
      if(pending) {
        this._pendingRequests.delete(payload.request_id);
        pending.resolve(
          new ResponseMessage(__type, payload.request_id, payload.result,
            payload.is_exception));
      }
    } else {
      const monitor = this._monitors.get(__type);
      if(monitor) {
        monitor.queue.push(monitor.deserialize(payload));
      } else {
        this._messages.push(Message.parse(json));
      }
    }
  }

  private onClose(): void {
    this.stopHeartbeat();
    const error = new PipeBrokenError();
    for(const pending of this._pendingRequests.values()) {
      pending.reject(error);
    }
    this._pendingRequests.clear();
    for(const monitor of this._monitors.values()) {
      monitor.queue.close(error);
    }
    this._monitors.clear();
    this._messages.close(error);
  }

  private _url: URL;
  private _heartbeatInterval: number;
  private _socket: WebSocket;
  private _heartbeatTimer: any;
  private _nextId: number;
  private _pendingRequests: Map<number, {
    resolve: (response: ResponseMessage) => void;
    reject: (error: Error) => void;
  }>;
  private _monitors: Map<string, {
    deserialize: (data: any) => any;
    queue: QueueWriter<any>;
  }>;
  private _messages: Queue<Message>;
}
