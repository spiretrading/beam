import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { ResponseMessage } from '../../source/services/response_message';

describe('ResponseMessage', () => {
  it('success_with_result', () => {
    const message = new ResponseMessage('Test.Service', 1, { value: 42 });
    assert.strictEqual(message.id, 1);
    assert.strictEqual(message.isException, false);
    assert.deepStrictEqual(message.result, { value: 42 });
  });

  it('success_without_result', () => {
    const message = new ResponseMessage('Test.Service', 1);
    assert.strictEqual(message.id, 1);
    assert.strictEqual(message.isException, false);
    assert.strictEqual(message.result, undefined);
  });

  it('exception_with_result', () => {
    const message =
      new ResponseMessage('Test.Service', 1, { message: 'failed' }, true);
    assert.strictEqual(message.isException, true);
    assert.deepStrictEqual(message.result, { message: 'failed' });
  });

  it('default_is_exception', () => {
    const message = new ResponseMessage('Test.Service', 1, { value: 42 });
    assert.strictEqual(message.isException, false);
  });

  it('type_appends_response', () => {
    const message = new ResponseMessage('Test.Service', 1);
    assert.strictEqual(message.type, 'Test.Service.Response');
  });

  it('to_json_with_result', () => {
    const message = new ResponseMessage('Test.Service', 1, { value: 42 });
    const json = message.toJson();
    assert.strictEqual(json.__type, 'Test.Service.Response');
    assert.strictEqual(json.request_id, 1);
    assert.strictEqual(json.is_exception, false);
    assert.deepStrictEqual(json.result, { value: 42 });
  });

  it('to_json_without_result', () => {
    const message = new ResponseMessage('Test.Service', 1);
    const json = message.toJson();
    assert.strictEqual(json.request_id, 1);
    assert.strictEqual(json.is_exception, false);
    assert.strictEqual(json.result, undefined);
  });

  it('to_json_exception', () => {
    const message =
      new ResponseMessage('Test.Service', 1, { message: 'error' }, true);
    const json = message.toJson();
    assert.strictEqual(json.request_id, 1);
    assert.strictEqual(json.is_exception, true);
    assert.deepStrictEqual(json.result, { message: 'error' });
  });
});
