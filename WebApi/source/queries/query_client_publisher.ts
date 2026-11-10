import { Map } from '../map';
import { CallbackQueueWriter } from '../queues/callback_queue_writer';
import { PipeBrokenError } from '../queues/pipe_broken_error';
import { QueueWriter } from '../queues/queue_writer';
import { Message, MessageType } from '../services/message';
import { ServiceProtocolClient } from '../services/service_protocol_client';
import { ServiceProtocolClientHandler } from
  '../services/service_protocol_client_handler';
import { ServiceRequest } from '../services/service_request';
import { BasicQuery } from './basic_query';
import { Evaluator } from './evaluator';
import { Expression } from './expression';
import { IndexedValue } from './indexed_value';
import { QueryResult } from './query_result';
import { Sequence } from './sequence';
import { SequencedValue } from './sequenced_value';
import { SequencedValuePublisher } from './sequenced_value_publisher';

interface Subscription<I, V> {
  index: I;
  queue: QueueWriter<SequencedValue<V>>;
  publisher: SequencedValuePublisher<I, V>;
  client: ServiceProtocolClient;
  id: number;
}

/**
 * Submits queries, publishes their updates, and recovers live subscriptions.
 * @param I - The query index type, supporting Beam's equality and hashing.
 * @param V - The type of value being queried.
 */
export class QueryClientPublisher<I, V> {

  /**
   * Constructs a QueryClientPublisher.
   * @param clientHandler - Provides connected service clients.
   * @param makeRequest - Constructs the request used to submit a query.
   * @param makeEndMessage - Constructs the message used to end a live query.
   * @param translateFilter - Translates a filter for values of type V.
   */
  constructor(clientHandler: ServiceProtocolClientHandler,
      makeRequest: (query: BasicQuery<I>) =>
        ServiceRequest<QueryResult<SequencedValue<V>>>,
      makeEndMessage: (index: I, id: number) => Message,
      translateFilter: (filter: Expression) => Evaluator<boolean>) {
    this._clientHandler = clientHandler;
    this._makeRequest = makeRequest;
    this._makeEndMessage = makeEndMessage;
    this._translateFilter = translateFilter;
    this._publishers = new Map();
    this._subscriptions = new Set();
    this._exception = null;
  }

  /**
   * Submits a query whose results omit sequence numbers.
   * @param query - The query to submit.
   * @param queue - Receives results and closes on completion or failure.
   * @return A queue writer whose close method cancels the query.
   */
  public submit(query: BasicQuery<I>, queue: QueueWriter<V>): QueueWriter<V> {
    return this.start(query, queue, value => value.value);
  }

  /**
   * Submits a query whose results include sequence numbers.
   * @param query - The query to submit.
   * @param queue - Receives results and closes on completion or failure.
   * @return A queue writer whose close method cancels the query.
   */
  public submitSequenced(query: BasicQuery<I>,
      queue: QueueWriter<SequencedValue<V>>): QueueWriter<SequencedValue<V>> {
    return this.start(query, queue, value => value);
  }

  /**
   * Publishes an indexed value to its live subscribers.
   * @param value - The indexed, sequenced value to publish.
   */
  public publish(value: SequencedValue<IndexedValue<V, I>>): void {
    const subscriptions = this._publishers.get(value.value.index);
    if(!subscriptions) {
      return;
    }
    const update = new SequencedValue(value.value.value, value.sequence);
    for(const subscription of Array.from(subscriptions)) {
      if(!this._subscriptions.has(subscription)) {
        continue;
      }
      try {
        subscription.publisher.push(update);
      } catch(error) {
        subscription.queue.close(error as Error);
      }
    }
  }

  /**
   * Recovers established live queries using a replacement client.
   * Call this from the connection handler's reconnect callback.
   * @param client - The connected client used for recovery.
   */
  public async recover(client: ServiceProtocolClient): Promise<void> {
    const recovering: {
      subscription: Subscription<I, V>, query: BasicQuery<I>
    }[] = [];
    for(const subscription of Array.from(this._subscriptions)) {
      if(!subscription.publisher || subscription.publisher.id === null) {
        continue;
      }
      subscription.id = null;
      subscription.client = client;
      try {
        recovering.push({
          subscription,
          query: subscription.publisher.beginRecovery()
        });
      } catch(error) {
        subscription.queue.close(error as Error);
      }
    }
    for(const {subscription, query} of recovering) {
      if(this._subscriptions.has(subscription)) {
        await this.initialize(query, subscription, client);
      }
    }
  }

  /**
   * Registers a message type whose updates are published automatically.
   * @param messageType - The service message carrying query updates.
   * @param extract - Extracts an indexed, sequenced value from the message.
   */
  public addMessageHandler<T extends Message>(messageType: MessageType<T>,
      extract: (message: T) => SequencedValue<IndexedValue<V, I>>): void {
    this._clientHandler.monitor(messageType, new CallbackQueueWriter<T>(
      message => this.publish(extract(message)), error => this.close(error)));
  }

  /**
   * Ends all queries and closes their queues, leaving the shared client open.
   * @param error - The error reported to subscribers.
   */
  public close(error?: Error): void {
    if(this._exception) {
      return;
    }
    this._exception = error ?? new PipeBrokenError();
    for(const subscription of Array.from(this._subscriptions)) {
      subscription.queue.close(this._exception);
    }
  }

  private start<T>(query: BasicQuery<I>, queue: QueueWriter<T>,
      convert: (value: SequencedValue<V>) => T): QueueWriter<T> {
    query = query.clone();
    const subscription: Subscription<I, V> = {
      index: query.index,
      queue: null,
      publisher: null,
      client: null,
      id: null
    };
    const writer = new CallbackQueueWriter<T>(value => queue.push(value),
      error => {
        this.remove(subscription);
        queue.close(error);
      });
    subscription.queue = {
      push: value => writer.push(convert(value)),
      close: error => writer.close(error)
    };
    if(this._exception) {
      writer.close(this._exception);
      return writer;
    }
    this._subscriptions.add(subscription);
    try {
      if(query.range.end instanceof Sequence &&
          query.range.end.equals(Sequence.LAST)) {
        subscription.publisher = new SequencedValuePublisher(query,
          this._translateFilter(query.filter), subscription.queue);
        let subscriptions = this._publishers.get(query.index);
        if(!subscriptions) {
          subscriptions = new Set();
          this._publishers.set(query.index, subscriptions);
        }
        subscriptions.add(subscription);
      }
    } catch(error) {
      writer.close(error as Error);
      return writer;
    }
    this.connect(query, subscription);
    return writer;
  }

  private async connect(query: BasicQuery<I>,
      subscription: Subscription<I, V>): Promise<void> {
    try {
      const client = await this._clientHandler.getClient();
      if(this._subscriptions.has(subscription)) {
        await this.initialize(query, subscription, client);
      }
    } catch(error) {
      subscription.queue.close(error as Error);
    }
  }

  private async initialize(query: BasicQuery<I>,
      subscription: Subscription<I, V>, client: ServiceProtocolClient):
      Promise<void> {
    subscription.client = client;
    try {
      const result = await client.sendRequest(this._makeRequest(query));
      subscription.id = result.id;
      if(subscription.id === -1) {
        subscription.id = null;
      }
      if(!this._subscriptions.has(subscription)) {
        this.end(subscription);
        return;
      }
      if(subscription.publisher) {
        subscription.publisher.pushSnapshot(result.snapshot);
        subscription.publisher.endSnapshot(subscription.id);
        if(subscription.id === null) {
          subscription.queue.close();
        }
      } else {
        for(const value of result.snapshot) {
          subscription.queue.push(value);
        }
        subscription.queue.close();
      }
    } catch(error) {
      subscription.queue.close(error as Error);
    }
  }

  private remove(subscription: Subscription<I, V>): void {
    if(!this._subscriptions.delete(subscription)) {
      return;
    }
    const subscriptions = this._publishers.get(subscription.index);
    if(subscriptions) {
      subscriptions.delete(subscription);
      if(subscriptions.size === 0) {
        this._publishers.delete(subscription.index);
      }
    }
    this.end(subscription);
  }

  private end(subscription: Subscription<I, V>): void {
    const id = subscription.id;
    subscription.id = null;
    if(id !== null && subscription.client) {
      try {
        subscription.client.sendMessage(
          this._makeEndMessage(subscription.index, id));
      } catch {
      }
    }
  }

  private _clientHandler: ServiceProtocolClientHandler;
  private _makeRequest: (query: BasicQuery<I>) =>
    ServiceRequest<QueryResult<SequencedValue<V>>>;
  private _makeEndMessage: (index: I, id: number) => Message;
  private _translateFilter: (filter: Expression) => Evaluator<boolean>;
  private _publishers: Map<I, Set<Subscription<I, V>>>;
  private _subscriptions: Set<Subscription<I, V>>;
  private _exception: Error;
}
