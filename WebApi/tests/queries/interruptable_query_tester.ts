import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { InterruptableQuery } from
  '../../source/queries/interruptable_query';
import { InterruptionPolicy } from
  '../../source/queries/interruption_policy';

describe('InterruptableQuery', () => {
  it('default_constructor', () => {
    assert.strictEqual(new InterruptableQuery().interruptionPolicy,
      InterruptionPolicy.BREAK_QUERY);
  });

  it('interruption_policy', () => {
    const query = new InterruptableQuery();
    query.interruptionPolicy = InterruptionPolicy.RECOVER_DATA;
    assert.strictEqual(
      query.interruptionPolicy, InterruptionPolicy.RECOVER_DATA);
  });

  it('equals', () => {
    const query = new InterruptableQuery(InterruptionPolicy.IGNORE_CONTINUE);
    assert.ok(query.equals(
      new InterruptableQuery(InterruptionPolicy.IGNORE_CONTINUE)));
    assert.ok(!query.equals(new InterruptableQuery()));
  });

  it('to_string', () => {
    assert.strictEqual(new InterruptableQuery().toString(), 'BREAK_QUERY');
  });

  it('round_trip', () => {
    const query = new InterruptableQuery(InterruptionPolicy.RECOVER_DATA);
    const json = query.toJson();
    assert.strictEqual(
      json.interruption_policy, InterruptionPolicy.RECOVER_DATA);
    assert.ok(InterruptableQuery.fromJson(json).equals(query));
  });
});
