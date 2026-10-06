import * as assert from 'node:assert/strict';
import { afterEach, beforeEach, describe, it } from 'node:test';
import { post } from '../../source/services/web_services';

class Request {
  public static instances: Request[] = [];
  public status = 200;
  public statusText = 'Failure';
  public responseText = '';
  public method: string;
  public url: string;
  public body: string;
  public sent = false;
  public headers: Record<string, string> = {};
  public onload: () => void;
  public onerror: () => void;
  public onabort: () => void;
  public ontimeout: () => void;

  public open(method: string, url: string): void {
    this.method = method;
    this.url = url;
    Request.instances.push(this);
  }

  public setRequestHeader(name: string, value: string): void {
    this.headers[name] = value;
  }

  public send(body?: string): void {
    this.sent = true;
    this.body = body;
  }

  public respond(status: number, body: string): void {
    this.status = status;
    this.responseText = body;
    this.onload();
  }
}

describe('post', () => {
  const original = globalThis.XMLHttpRequest;
  beforeEach(() => {
    Request.instances = [];
    globalThis.XMLHttpRequest = Request as unknown as typeof XMLHttpRequest;
  });
  afterEach(() => { globalThis.XMLHttpRequest = original; });

  it('posts_json_and_reads_empty_success', async () => {
    const result = post('/test', {id: 'job'});
    const request = Request.instances[0];
    assert.equal(request.method, 'POST');
    assert.equal(request.body, '{"id":"job"}');
    assert.equal(request.headers['Content-Type'], 'application/json');
    request.respond(204, '');
    assert.equal(await result, undefined);
  });

  it('sends_without_parameters_and_parses_json', async () => {
    const result = post('/test');
    const request = Request.instances[0];
    assert.equal(request.body, undefined);
    assert.ok(request.sent);
    assert.deepEqual(request.headers, {});
    request.respond(200, '{"id":"job"}');
    assert.deepEqual(await result, {id: 'job'});
  });

  it('rejects_http_errors', async () => {
    for(const body of ['{"message":"denied"}', '{"error":"denied"}',
        '"denied"', 'denied']) {
      const result = post('/test', {});
      Request.instances.at(-1).respond(403, body);
      await assert.rejects(result, (error: any) =>
        error.code === 403 && error.message === 'denied');
    }
  });

  it('rejects_malformed_json', async () => {
    const result = post('/test', {});
    Request.instances.at(-1).respond(200, '{invalid');
    await assert.rejects(result, SyntaxError);
  });

  it('rejects_network_abort_and_timeout', async () => {
    for(const event of ['onerror', 'onabort', 'ontimeout'] as const) {
      const result = post('/test', {});
      Request.instances.at(-1)[event]();
      await assert.rejects(result, (error: any) =>
        typeof error.message === 'string' && error.message.length !== 0);
    }
  });
});
