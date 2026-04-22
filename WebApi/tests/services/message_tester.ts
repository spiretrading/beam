import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { Message } from '../../source/services/message';

class TestMessage extends Message {
  public static fromJson(data: any): TestMessage {
    return new TestMessage(data.value, data.label);
  }

  constructor(public readonly value: number, public readonly label: string) {
    super('Test.Message', { value, label });
  }
}

class VersionedMessage extends Message {
  public static fromJson(data: any): VersionedMessage {
    return new VersionedMessage(data.name);
  }

  constructor(public readonly name: string) {
    super('Test.Versioned', { name }, 2);
  }
}

describe('Message', () => {
  it('to_json', () => {
    const message = new TestMessage(42, 'hello');
    const json = message.toJson();
    assert.strictEqual(json.__type, 'Test.Message');
    assert.strictEqual(json.__version, 0);
    assert.strictEqual(json.value, 42);
    assert.strictEqual(json.label, 'hello');
  });

  it('to_json_no_extra_keys', () => {
    const message = new TestMessage(1, 'a');
    const json = message.toJson();
    const keys = Object.keys(json).sort();
    assert.deepStrictEqual(keys, ['__type', '__version', 'label', 'value']);
  });

  it('to_json_with_version', () => {
    const message = new VersionedMessage('test');
    const json = message.toJson();
    assert.strictEqual(json.__type, 'Test.Versioned');
    assert.strictEqual(json.__version, 2);
    assert.strictEqual(json.name, 'test');
  });

  it('default_version', () => {
    const message = new TestMessage(0, '');
    assert.strictEqual(message.version, 0);
  });

  it('type', () => {
    const message = new TestMessage(0, '');
    assert.strictEqual(message.type, 'Test.Message');
  });

  it('data', () => {
    const message = new TestMessage(42, 'hello');
    assert.deepStrictEqual(message.data, { value: 42, label: 'hello' });
  });

  it('from_json', () => {
    const json = {
      __type: 'Test.Message',
      __version: 0,
      value: 42,
      label: 'hello'
    };
    const message = Message.fromJson(TestMessage, json);
    assert.ok(message instanceof TestMessage);
    assert.strictEqual(message.value, 42);
    assert.strictEqual(message.label, 'hello');
    assert.strictEqual(message.type, 'Test.Message');
  });

  it('from_json_strips_metadata', () => {
    const json = {
      __type: 'Test.Message',
      __version: 0,
      value: 99,
      label: 'stripped'
    };
    const message = Message.fromJson(TestMessage, json);
    assert.strictEqual(message.value, 99);
    assert.strictEqual(message.label, 'stripped');
  });

  it('from_json_versioned', () => {
    const json = {
      __type: 'Test.Versioned',
      __version: 2,
      name: 'test'
    };
    const message = Message.fromJson(VersionedMessage, json);
    assert.ok(message instanceof VersionedMessage);
    assert.strictEqual(message.name, 'test');
    assert.strictEqual(message.version, 2);
  });

  it('round_trip', () => {
    const original = new TestMessage(42, 'hello');
    const json = original.toJson();
    const restored = Message.fromJson(TestMessage, json);
    assert.strictEqual(restored.value, original.value);
    assert.strictEqual(restored.label, original.label);
    assert.strictEqual(restored.type, original.type);
    assert.strictEqual(restored.version, original.version);
  });

  it('round_trip_versioned', () => {
    const original = new VersionedMessage('test');
    const json = original.toJson();
    const restored = Message.fromJson(VersionedMessage, json);
    assert.strictEqual(restored.name, original.name);
    assert.strictEqual(restored.type, original.type);
    assert.strictEqual(restored.version, original.version);
  });
});
