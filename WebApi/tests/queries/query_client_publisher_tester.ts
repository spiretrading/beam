import * as assert from 'node:assert/strict';
import { setImmediate } from 'node:timers/promises';
import { afterEach, beforeEach, describe, it } from 'node:test';
import { BasicQuery, CallbackQueueWriter, ConstantExpression, DirectoryEntry,
  ExpressionTranslationError, FunctionExpression, IndexedValue, IntValue,
  InterruptionPolicy, makeGreater, Message, ParameterExpression,
  PipeBrokenError, QueryClientPublisher, QueryInterruptedError, QueryResult,
  QueryType, Queue, Range, Sequence, SequencedValue, ServiceProtocolClient,
  ServiceProtocolClientHandler, ServiceRequest, translate } from '../../source';

interface PendingRequest {
  query: BasicQuery<DirectoryEntry>;
  resolve: (result: QueryResult<SequencedValue<number>>) => void;
  reject: (error: Error) => void;
}

class TestRequest implements
    ServiceRequest<QueryResult<SequencedValue<number>>> {
  public readonly service: string;
  public readonly query: BasicQuery<DirectoryEntry>;

  constructor(query: BasicQuery<DirectoryEntry>) {
    this.service = 'Test.Query';
    this.query = query;
  }

  public toJson(): any {
    return {query: this.query.toJson()};
  }

  public parseResponse(value: any): QueryResult<SequencedValue<number>> {
    return value;
  }
}

class EndMessage extends Message {
  constructor(index: DirectoryEntry, id: number) {
    super('Test.End', {index, id});
  }
}

class Update extends Message {
  public static readonly TYPE = 'Test.Update';

  public static fromJson(value: any): Update {
    return new Update(value.value);
  }

  constructor(value: SequencedValue<IndexedValue<number, DirectoryEntry>>) {
    super(Update.TYPE, {value});
  }
}

class TestClient extends ServiceProtocolClient {
  public requests: Queue<PendingRequest>;
  public sent: Queue<Message>;
  public messages: Queue<Message>;
  public isClosed: boolean;

  constructor() {
    super(new URL('ws://localhost/test'));
    this.requests = new Queue();
    this.sent = new Queue();
    this.messages = new Queue();
    this.isClosed = false;
    this._pending = new Set();
  }

  public async sendRequest<R>(request: ServiceRequest<R>): Promise<R> {
    if(this.isClosed) {
      throw new PipeBrokenError();
    }
    return new Promise<R>((resolve, reject) => {
      const pending: PendingRequest = {
        query: BasicQuery.fromJson<DirectoryEntry>(DirectoryEntry,
          request.toJson().query),
        resolve: result => {
          this._pending.delete(pending);
          resolve(request.parseResponse(result));
        },
        reject: error => {
          this._pending.delete(pending);
          reject(error);
        }
      };
      this._pending.add(pending);
      this.requests.push(pending);
    });
  }

  public sendMessage(message: Message): void {
    if(this.isClosed) {
      throw new PipeBrokenError();
    }
    this.sent.push(message);
  }

  public readMessage(): Promise<Message> {
    return this.messages.pop();
  }

  public close(): void {
    this.isClosed = true;
    this.messages.close();
    for(const pending of this._pending) {
      pending.reject(new PipeBrokenError());
    }
  }

  private _pending: Set<PendingRequest>;
}

describe('QueryClientPublisher', () => {
  const account = DirectoryEntry.makeAccount(7, 'account');
  let handlers: ServiceProtocolClientHandler[];
  let publishers: QueryClientPublisher<DirectoryEntry, number>[];
  let created: Queue<TestClient>;

  beforeEach(() => {
    handlers = [];
    publishers = [];
    created = new Queue();
  });

  afterEach(() => {
    for(const publisher of publishers) {
      publisher.close();
    }
    for(const handler of handlers) {
      handler.close();
    }
  });

  function makePublisher() {
    let publisher: QueryClientPublisher<DirectoryEntry, number>;
    const handler = new ServiceProtocolClientHandler(async () => {
      const client = new TestClient();
      created.push(client);
      return client;
    }, client => publisher.recover(client));
    publisher = new QueryClientPublisher(handler,
      query => new TestRequest(query),
      (index, id) => new EndMessage(index, id),
      filter => translate<boolean>(filter, QueryType.INT));
    publisher.addMessageHandler(Update, message => message.data.value);
    handlers.push(handler);
    publishers.push(publisher);
    return {publisher, handler};
  }

  it('snapshot_overlap_and_index_equality', async () => {
    const {publisher} = makePublisher();
    const queue = new Queue<SequencedValue<number>>();
    const query = BasicQuery.makeCurrentQuery(account);
    publisher.submitSequenced(query, queue);
    query.index = DirectoryEntry.makeAccount(99, 'changed');
    const client = await created.pop();
    const request = await client.requests.pop();
    assert.ok(request.query.index.equals(account));
    const equivalent = DirectoryEntry.makeAccount(7, 'account');
    client.messages.push(new Update(new SequencedValue(
      new IndexedValue(20, equivalent), new Sequence(2n))));
    client.messages.push(new Update(new SequencedValue(
      new IndexedValue(30, equivalent), new Sequence(3n))));
    await setImmediate();
    assert.equal(queue.tryPop(), null);
    request.resolve(new QueryResult(0, [
      new SequencedValue(10, new Sequence(1n)),
      new SequencedValue(20, new Sequence(2n))]));
    for(const expected of [10, 20, 30]) {
      assert.equal((await queue.pop()).value, expected);
    }
    publisher.publish(new SequencedValue(
      new IndexedValue(20, equivalent), new Sequence(2n)));
    publisher.publish(new SequencedValue(
      new IndexedValue(40, query.index), new Sequence(4n)));
    assert.equal(queue.tryPop(), null);
  });

  it('independent_filters_and_cancellation', async () => {
    const {publisher} = makePublisher();
    const first = new Queue<number>();
    const second = new Queue<number>();
    const query = BasicQuery.makeCurrentQuery(account);
    const writer = publisher.submit(query, first);
    query.filter = makeGreater(new ParameterExpression(0, QueryType.INT),
      new ConstantExpression(new IntValue(5)));
    publisher.submit(query, second);
    const client = await created.pop();
    (await client.requests.pop()).resolve(new QueryResult(1, []));
    (await client.requests.pop()).resolve(new QueryResult(2, []));
    await setImmediate();
    publisher.publish(new SequencedValue(
      new IndexedValue(3, account), new Sequence(1n)));
    assert.equal(await first.pop(), 3);
    assert.equal(second.tryPop(), null);
    publisher.publish(new SequencedValue(
      new IndexedValue(7, account), new Sequence(2n)));
    assert.equal(await first.pop(), 7);
    assert.equal(await second.pop(), 7);
    writer.close();
    writer.close();
    assert.equal((await client.sent.pop()).data.id, 1);
    assert.equal(client.sent.tryPop(), null);
    publisher.publish(new SequencedValue(
      new IndexedValue(9, account), new Sequence(3n)));
    assert.equal(await second.pop(), 9);
    await assert.rejects(first.pop(), PipeBrokenError);
  });

  it('historical_snapshot_completes', async () => {
    const {publisher} = makePublisher();
    const queue = new Queue<number>();
    publisher.submit(BasicQuery.makeLatestQuery(account), queue);
    const client = await created.pop();
    (await client.requests.pop()).resolve(new QueryResult(-1, [
      new SequencedValue(10, new Sequence(1n))]));
    assert.equal(await queue.pop(), 10);
    await assert.rejects(queue.pop(), PipeBrokenError);
    assert.equal(client.sent.tryPop(), null);
  });

  it('cancellation_before_request_and_before_response', async () => {
    const {publisher} = makePublisher();
    const first = new Queue<number>();
    const query = BasicQuery.makeCurrentQuery(account);
    publisher.submit(query, first).close();
    const client = await created.pop();
    await setImmediate();
    assert.equal(client.requests.tryPop(), null);
    await assert.rejects(first.pop(), PipeBrokenError);
    const second = new Queue<number>();
    const writer = publisher.submit(query, second);
    const request = await client.requests.pop();
    writer.close();
    request.resolve(new QueryResult(42, [
      new SequencedValue(10, new Sequence(1n))]));
    assert.equal((await client.sent.pop()).data.id, 42);
    await assert.rejects(second.pop(), PipeBrokenError);
  });

  it('close_ends_queries_without_closing_shared_client', async () => {
    const {publisher, handler} = makePublisher();
    const first = new Queue<number>();
    const second = new Queue<number>();
    publisher.submit(BasicQuery.makeCurrentQuery(account), first);
    publisher.submit(BasicQuery.makeLatestQuery(account), second);
    const client = await created.pop();
    const active = await client.requests.pop();
    const pending = await client.requests.pop();
    active.resolve(new QueryResult(7, []));
    await setImmediate();
    publisher.close();
    assert.equal((await client.sent.pop()).data.id, 7);
    await assert.rejects(first.pop(), PipeBrokenError);
    await assert.rejects(second.pop(), PipeBrokenError);
    pending.resolve(new QueryResult(-1, []));
    assert.equal(await handler.getClient(), client);
    assert.ok(!client.isClosed);
    const later = new Queue<number>();
    publisher.submit(BasicQuery.makeCurrentQuery(account), later);
    await assert.rejects(later.pop(), PipeBrokenError);
    assert.equal(client.requests.tryPop(), null);
  });

  it('translation_request_and_delivery_failures', async () => {
    const {publisher} = makePublisher();
    const query = BasicQuery.makeCurrentQuery(account);
    query.filter = new FunctionExpression('unsupported', QueryType.BOOL, []);
    const invalid = new Queue<number>();
    publisher.submit(query, invalid);
    await assert.rejects(invalid.pop(), ExpressionTranslationError);
    assert.equal(created.tryPop(), null);
    query.filter = ConstantExpression.TRUE;
    const failed = new Queue<number>();
    publisher.submit(query, failed);
    const client = await created.pop();
    const error = new Error('Request failed.');
    (await client.requests.pop()).reject(error);
    await assert.rejects(failed.pop(), value => value === error);
    const errors = new Queue<Error>();
    publisher.submit(query, new CallbackQueueWriter<number>(() => {
      throw error;
    }, value => errors.push(value)));
    (await client.requests.pop()).resolve(new QueryResult(99, [
      new SequencedValue(10, new Sequence(1n))]));
    assert.equal(await errors.pop(), error);
    assert.equal((await client.sent.pop()).data.id, 99);
  });

  it('live_query_without_subscription_completes', async () => {
    const {publisher} = makePublisher();
    const queue = new Queue<number>();
    publisher.submit(BasicQuery.makeCurrentQuery(account), queue);
    const client = await created.pop();
    (await client.requests.pop()).resolve(new QueryResult(-1, [
      new SequencedValue(10, new Sequence(1n))]));
    assert.equal(await queue.pop(), 10);
    await assert.rejects(queue.pop(), PipeBrokenError);
    publisher.publish(new SequencedValue(
      new IndexedValue(20, account), new Sequence(2n)));
    assert.equal(client.sent.tryPop(), null);
  });

  it('recovery_buffers_all_queries_before_sending_requests', async () => {
    const {publisher, handler} = makePublisher();
    const query = BasicQuery.makeCurrentQuery(account);
    query.interruptionPolicy = InterruptionPolicy.RECOVER_DATA;
    const queues = [new Queue<number>(), new Queue<number>()];
    for(const queue of queues) {
      publisher.submit(query, queue);
    }
    const first = await created.pop();
    for(let i = 0; i < queues.length; ++i) {
      (await first.requests.pop()).resolve(new QueryResult(10 + i, [
        new SequencedValue(5, new Sequence(5n))]));
      assert.equal(await queues[i].pop(), 5);
    }
    first.close();
    const second = await created.pop();
    const recovering = await second.requests.pop();
    assert.ok(recovering.query.range.equals(
      new Range(new Sequence(6n), Sequence.LAST)));
    second.messages.push(new Update(new SequencedValue(
      new IndexedValue(8, account), new Sequence(8n))));
    await setImmediate();
    assert.equal(queues[0].tryPop(), null);
    assert.equal(queues[1].tryPop(), null);
    const snapshot = [new SequencedValue(6, new Sequence(6n)),
      new SequencedValue(7, new Sequence(7n))];
    recovering.resolve(new QueryResult(20, snapshot));
    const next = await second.requests.pop();
    assert.ok(next.query.range.equals(
      new Range(new Sequence(6n), Sequence.LAST)));
    next.resolve(new QueryResult(21, snapshot));
    for(const queue of queues) {
      for(const expected of [6, 7, 8]) {
        assert.equal(await queue.pop(), expected);
      }
      assert.equal(queue.tryPop(), null);
    }
    assert.equal(await handler.getClient(), second);
  });

  it('recovery_respects_ignore_and_break_policies', async () => {
    const {publisher} = makePublisher();
    const continuing = new Queue<number>();
    const breaking = new Queue<number>();
    const query = BasicQuery.makeCurrentQuery(account);
    publisher.submit(query, continuing);
    query.interruptionPolicy = InterruptionPolicy.BREAK_QUERY;
    publisher.submit(query, breaking);
    const first = await created.pop();
    (await first.requests.pop()).resolve(new QueryResult(1, []));
    (await first.requests.pop()).resolve(new QueryResult(2, []));
    await setImmediate();
    first.close();
    const second = await created.pop();
    const recovery = await second.requests.pop();
    await assert.rejects(breaking.pop(), QueryInterruptedError);
    assert.ok(recovery.query.range.equals(Range.REAL_TIME));
    assert.equal(second.requests.tryPop(), null);
    recovery.resolve(new QueryResult(3, [
      new SequencedValue(50, new Sequence(50n))]));
    assert.equal(await continuing.pop(), 50);
  });

  it('cancellation_during_recovery_ends_replacement_id', async () => {
    const {publisher, handler} = makePublisher();
    const queue = new Queue<number>();
    const writer = publisher.submit(
      BasicQuery.makeCurrentQuery(account), queue);
    const first = await created.pop();
    (await first.requests.pop()).resolve(new QueryResult(10, []));
    await setImmediate();
    first.close();
    const second = await created.pop();
    const recovery = await second.requests.pop();
    writer.close();
    assert.equal(second.sent.tryPop(), null);
    recovery.resolve(new QueryResult(20, []));
    assert.equal((await second.sent.pop()).data.id, 20);
    assert.equal(first.sent.tryPop(), null);
    await assert.rejects(queue.pop(), PipeBrokenError);
    assert.equal(await handler.getClient(), second);
  });

  it('failed_recovery_does_not_stop_other_queries', async () => {
    const {publisher, handler} = makePublisher();
    const firstQueue = new Queue<number>();
    const secondQueue = new Queue<number>();
    publisher.submit(BasicQuery.makeCurrentQuery(account), firstQueue);
    publisher.submit(BasicQuery.makeCurrentQuery(account), secondQueue);
    const first = await created.pop();
    (await first.requests.pop()).resolve(new QueryResult(1, []));
    (await first.requests.pop()).resolve(new QueryResult(2, []));
    await setImmediate();
    first.close();
    const second = await created.pop();
    const error = new Error('Recovery request rejected.');
    (await second.requests.pop()).reject(error);
    await assert.rejects(firstQueue.pop(), value => value === error);
    (await second.requests.pop()).resolve(new QueryResult(3, [
      new SequencedValue(42, new Sequence(42n))]));
    assert.equal(await secondQueue.pop(), 42);
    assert.equal(await handler.getClient(), second);
  });

  it('terminal_connection_failure_closes_subscribers', async () => {
    const {publisher, handler} = makePublisher();
    const queue = new Queue<number>();
    publisher.submit(BasicQuery.makeCurrentQuery(account), queue);
    const client = await created.pop();
    (await client.requests.pop()).resolve(new QueryResult(1, []));
    await setImmediate();
    const error = new Error('Connection stopped.');
    handler.close(error);
    await assert.rejects(queue.pop(), value => value === error);
    const later = new Queue<number>();
    publisher.submit(BasicQuery.makeCurrentQuery(account), later);
    await assert.rejects(later.pop(), value => value === error);
  });
});
