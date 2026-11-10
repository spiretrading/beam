import * as assert from 'node:assert/strict';
import { setImmediate } from 'node:timers/promises';
import { afterEach, beforeEach, describe, it } from 'node:test';
import { ConnectError, Message, PipeBrokenError, Queue, ServiceProtocolClient,
  ServiceProtocolClientHandler } from '../../source';

class TestWebSocket {
  public static instances: Queue<TestWebSocket>;
  public onopen: () => void;
  public onerror: () => void;
  public onclose: () => void;
  public onmessage: (event: {data: ArrayBuffer}) => void;
  public requests: Queue<any>;
  public isClosed: boolean;

  constructor(url: string) {
    this.onopen = null;
    this.onerror = null;
    this.onclose = null;
    this.onmessage = null;
    this.requests = new Queue();
    this.isClosed = false;
    TestWebSocket.instances.push(this);
  }

  public send(buffer: ArrayBuffer): void {
    if(this.isClosed) {
      throw new PipeBrokenError();
    }
    this.requests.push(JSON.parse(new TextDecoder().decode(buffer.slice(4))));
  }

  public receive(value: any): void {
    const payload = new TextEncoder().encode(JSON.stringify(value));
    const buffer = new ArrayBuffer(4 + payload.length);
    new DataView(buffer).setUint32(0, payload.length, true);
    new Uint8Array(buffer, 4).set(payload);
    this.onmessage({data: buffer});
  }

  public respond(request: any, result: any): void {
    this.receive({
      __type: 'Test.Response',
      request_id: request.request_id,
      is_exception: false,
      result
    });
  }

  public disconnect(): void {
    this.isClosed = true;
    this.onclose?.();
  }

  public close(): void {
    this.isClosed = true;
  }
}

class Update extends Message {
  public static readonly TYPE = 'Test.Update';

  public static fromJson(value: any): Update {
    return new Update(value.value);
  }

  constructor(value: number) {
    super(Update.TYPE, {value});
  }
}

const REQUEST = {
  service: 'Test.Request',
  toJson: () => ({}),
  parseResponse: (value: any) => value
};

describe('ServiceProtocolClientHandler', () => {
  const original = globalThis.WebSocket;
  let handlers: ServiceProtocolClientHandler[];
  let clients: ServiceProtocolClient[];

  beforeEach(() => {
    handlers = [];
    clients = [];
    TestWebSocket.instances = new Queue();
    globalThis.WebSocket = TestWebSocket as unknown as typeof WebSocket;
  });

  afterEach(() => {
    for(const handler of handlers) {
      handler.close();
    }
    for(const client of clients) {
      client.close();
    }
    globalThis.WebSocket = original;
  });

  async function buildClient(): Promise<ServiceProtocolClient> {
    const client = new ServiceProtocolClient(new URL('ws://localhost/test'));
    clients.push(client);
    await client.open();
    return client;
  }

  it('concurrent_callers_and_message_dispatch', async () => {
    const handler = new ServiceProtocolClientHandler(buildClient);
    handlers.push(handler);
    const first = handler.getClient();
    assert.equal(handler.getClient(), first);
    const socket = await TestWebSocket.instances.pop();
    socket.onopen();
    const client = await first;
    assert.equal(await handler.getClient(), client);
    assert.equal(TestWebSocket.instances.tryPop(), null);
    const updates = new Queue<Update>();
    handler.monitor(Update, updates);
    socket.receive({__type: 'Test.Unknown'});
    socket.receive(new Update(7).toJson());
    assert.equal((await updates.pop()).data.value, 7);
    const response = client.sendRequest(REQUEST);
    socket.respond(await socket.requests.pop(), 42);
    assert.equal(await response, 42);
  });

  it('reconnect_dispatches_during_recovery_without_replaying_requests',
      async () => {
    let recoveries = 0;
    const handler = new ServiceProtocolClientHandler(buildClient,
      async client => {
        ++recoveries;
        await client.sendRequest(REQUEST);
      });
    handlers.push(handler);
    const updates = new Queue<Update>();
    handler.monitor(Update, updates);
    const opening = handler.getClient();
    let socket = await TestWebSocket.instances.pop();
    socket.onopen();
    const initial = await opening;
    assert.equal(recoveries, 0);
    const pending = initial.sendRequest(REQUEST);
    await socket.requests.pop();
    socket.disconnect();
    await assert.rejects(pending, PipeBrokenError);
    for(let i = 1; i <= 2; ++i) {
      socket = await TestWebSocket.instances.pop();
      const connecting = handler.getClient();
      let isReady = false;
      connecting.then(() => { isReady = true; });
      socket.onopen();
      const recovery = await socket.requests.pop();
      assert.equal(recoveries, i);
      assert.ok(!isReady);
      assert.equal(socket.requests.tryPop(), null);
      socket.receive(new Update(i).toJson());
      assert.equal((await updates.pop()).data.value, i);
      socket.respond(recovery, true);
      const replacement = await connecting;
      assert.notEqual(replacement, initial);
      assert.equal(await handler.getClient(), replacement);
      assert.ok(!updates.isBroken());
      if(i === 1) {
        socket.disconnect();
      }
    }
  });

  it('retry_connection_errors_and_stop_on_other_errors', async () => {
    let attempts = 0;
    const fatal = new Error('Authentication rejected.');
    const handler = new ServiceProtocolClientHandler(async () => {
      ++attempts;
      if(attempts <= 2) {
        throw new ConnectError();
      }
      if(attempts > 3) {
        throw fatal;
      }
      return buildClient();
    }, () => {}, 0);
    handlers.push(handler);
    const updates = new Queue<Update>();
    handler.monitor(Update, updates);
    const opening = handler.getClient();
    const socket = await TestWebSocket.instances.pop();
    assert.equal(attempts, 3);
    socket.onopen();
    await opening;
    socket.disconnect();
    await assert.rejects(updates.pop(), error => error === fatal);
    await assert.rejects(handler.getClient(), error => error === fatal);
    assert.equal(attempts, 4);
  });

  it('failed_socket_open_retries_and_close_ends_monitors', async () => {
    const handler = new ServiceProtocolClientHandler(buildClient, () => {}, 0);
    handlers.push(handler);
    const updates = new Queue<Update>();
    handler.monitor(Update, updates);
    const opening = handler.getClient();
    const first = await TestWebSocket.instances.pop();
    first.onerror();
    const second = await TestWebSocket.instances.pop();
    assert.ok(first.isClosed);
    assert.equal(handler.getClient(), opening);
    second.onopen();
    await opening;
    second.receive(new Update(9).toJson());
    assert.equal((await updates.pop()).data.value, 9);
    handler.close();
    await assert.rejects(updates.pop(), PipeBrokenError);
    assert.ok(second.isClosed);
    await setImmediate();
    assert.equal(TestWebSocket.instances.tryPop(), null);
  });

  it('recovery_failure_stops_handler', async () => {
    const fatal = new Error('Recovery failed.');
    const handler = new ServiceProtocolClientHandler(buildClient, () => {
      throw fatal;
    });
    handlers.push(handler);
    const updates = new Queue<Update>();
    handler.monitor(Update, updates);
    const opening = handler.getClient();
    const first = await TestWebSocket.instances.pop();
    first.onopen();
    await opening;
    first.disconnect();
    const second = await TestWebSocket.instances.pop();
    const waiting = handler.getClient();
    second.onopen();
    await assert.rejects(waiting, error => error === fatal);
    await assert.rejects(updates.pop(), error => error === fatal);
    assert.ok(second.isClosed);
    assert.equal(TestWebSocket.instances.tryPop(), null);
  });

  it('close_during_construction_disposes_late_client', async () => {
    const handler = new ServiceProtocolClientHandler(buildClient);
    handlers.push(handler);
    const opening = handler.getClient();
    const socket = await TestWebSocket.instances.pop();
    handler.close();
    await assert.rejects(opening, PipeBrokenError);
    socket.onopen();
    await setImmediate();
    assert.ok(socket.isClosed);
    await assert.rejects(handler.getClient(), PipeBrokenError);
    const updates = new Queue<Update>();
    handler.monitor(Update, updates);
    await assert.rejects(updates.pop(), PipeBrokenError);
  });

  it('close_cancels_retry_delay', async () => {
    let attempts = 0;
    const handler = new ServiceProtocolClientHandler(async () => {
      ++attempts;
      throw new ConnectError();
    }, () => {}, 60000);
    handlers.push(handler);
    const opening = handler.getClient();
    await setImmediate();
    handler.close();
    await assert.rejects(opening, PipeBrokenError);
    await setImmediate();
    assert.equal(attempts, 1);
  });

  it('close_during_recovery', async () => {
    const handler = new ServiceProtocolClientHandler(buildClient,
      async client => {
        await client.sendRequest(REQUEST);
      });
    handlers.push(handler);
    const opening = handler.getClient();
    const first = await TestWebSocket.instances.pop();
    first.onopen();
    await opening;
    first.disconnect();
    const second = await TestWebSocket.instances.pop();
    const waiting = handler.getClient();
    second.onopen();
    await second.requests.pop();
    handler.close();
    await assert.rejects(waiting, PipeBrokenError);
    assert.ok(second.isClosed);
    await setImmediate();
    assert.equal(TestWebSocket.instances.tryPop(), null);
  });

  it('opening_failure_and_cancellation', async () => {
    for(const action of ['disconnect', 'error', 'close']) {
      const client = new ServiceProtocolClient(new URL('ws://localhost/test'));
      clients.push(client);
      const opening = client.open();
      const socket = await TestWebSocket.instances.pop();
      if(action === 'disconnect') {
        socket.disconnect();
      } else if(action === 'error') {
        socket.onerror();
      } else {
        client.close();
      }
      await assert.rejects(opening, ConnectError);
      assert.ok(socket.isClosed);
    }
  });

  it('failed_send_does_not_leave_a_pending_request', async () => {
    const opening = buildClient();
    const socket = await TestWebSocket.instances.pop();
    socket.onopen();
    const client = await opening;
    socket.isClosed = true;
    await assert.rejects(client.sendRequest(REQUEST), PipeBrokenError);
    client.close();
    await setImmediate();
  });
});
