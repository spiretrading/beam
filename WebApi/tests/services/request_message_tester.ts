import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { RequestMessage } from '../../source/services/request_message';

describe('RequestMessage', () => {
  it('to_json', () => {
    const message = new RequestMessage('Test.Service', 1,
      { account: 'abc', value: 42 });
    const json = message.toJson();
    assert.strictEqual(json.__type, 'Test.Service.Request');
    assert.strictEqual(json.__version, 0);
    assert.strictEqual(json.request_id, 1);
    assert.deepStrictEqual(json.parameters, { account: 'abc', value: 42 });
  });

  it('type', () => {
    const message = new RequestMessage('Test.Service', 1, {});
    assert.strictEqual(message.type, 'Test.Service.Request');
  });

  it('id', () => {
    const message = new RequestMessage('Test.Service', 5, {});
    assert.strictEqual(message.id, 5);
  });

  it('empty_parameters_omitted', () => {
    const message = new RequestMessage('Test.Service', 1, {});
    const json = message.toJson();
    assert.strictEqual(json.__type, 'Test.Service.Request');
    assert.strictEqual(json.__version, 0);
    assert.strictEqual(json.request_id, 1);
    assert.strictEqual(json.parameters, undefined);
  });

  it('non_empty_parameters_included', () => {
    const message = new RequestMessage('Test.Service', 1, { key: 'value' });
    const json = message.toJson();
    assert.deepStrictEqual(json.parameters, { key: 'value' });
  });

  it('different_ids', () => {
    const a = new RequestMessage('Test.Service', 1, {});
    const b = new RequestMessage('Test.Service', 2, {});
    assert.notStrictEqual(a.id, b.id);
  });

  it('inherits_version', () => {
    const message = new RequestMessage('Test.Service', 1, {});
    assert.strictEqual(message.version, 0);
  });
});
