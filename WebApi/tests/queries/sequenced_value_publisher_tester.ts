import * as assert from 'node:assert';
import { describe, it } from 'node:test';
import { BasicQuery, ConstantExpression, DateTime, IntValue,
  InterruptionPolicy, makeDivision, makeGreater, ParameterExpression,
  PipeBrokenError, QueryInterruptedError, QueryType, Queue, Range, Sequence,
  SequencedValue, SequencedValuePublisher, SnapshotLimit, translate } from
  '../../source';

function makePublisher(query: BasicQuery<string>) {
  const queue = new Queue<SequencedValue<number>>();
  const publisher = new SequencedValuePublisher(query,
    translate<boolean>(query.filter, QueryType.INT), queue);
  return {queue, publisher};
}

describe('SequencedValuePublisher', () => {
  it('snapshot_and_buffered_updates', () => {
    const {queue, publisher} = makePublisher(
      BasicQuery.makeCurrentQuery('test'));
    const first = new SequencedValue(10, new Sequence(1n));
    const second = new SequencedValue(20, new Sequence(2n));
    const third = new SequencedValue(30, new Sequence(3n));
    assert.strictEqual(publisher.id, null);
    publisher.beginSnapshot();
    publisher.push(second);
    publisher.push(third);
    assert.strictEqual(queue.tryPop(), null);
    publisher.pushSnapshot(new Set([first, second]));
    assert.deepStrictEqual(queue.tryPop(), first);
    assert.deepStrictEqual(queue.tryPop(), second);
    assert.strictEqual(queue.tryPop(), null);
    publisher.endSnapshot(0);
    assert.strictEqual(publisher.id, 0);
    assert.deepStrictEqual(queue.tryPop(), third);
    assert.strictEqual(queue.tryPop(), null);
    publisher.push(first);
    publisher.push(third);
    assert.strictEqual(queue.tryPop(), null);
    const fourth = new SequencedValue(40, new Sequence(4n));
    publisher.push(fourth);
    assert.deepStrictEqual(queue.tryPop(), fourth);
  });

  it('empty_snapshot_and_snapshot_restart', () => {
    const {queue, publisher} = makePublisher(
      BasicQuery.makeCurrentQuery('test'));
    const first = new SequencedValue(10, Sequence.FIRST);
    publisher.push(first);
    publisher.pushSnapshot([]);
    publisher.endSnapshot(7);
    assert.deepStrictEqual(queue.tryPop(), first);
    publisher.beginSnapshot();
    assert.strictEqual(publisher.id, null);
    const second = new SequencedValue(20, new Sequence(1n));
    publisher.push(second);
    publisher.pushSnapshot([first]);
    assert.strictEqual(queue.tryPop(), null);
    publisher.endSnapshot(8);
    assert.strictEqual(publisher.id, 8);
    assert.deepStrictEqual(queue.tryPop(), second);
    assert.strictEqual(queue.tryPop(), null);
  });

  it('filters_snapshot_buffered_and_live_values', () => {
    const query = BasicQuery.makeCurrentQuery('test');
    query.filter = makeGreater(makeDivision(
      new ConstantExpression(new IntValue(10)),
      new ParameterExpression(0, QueryType.INT)),
      new ConstantExpression(new IntValue(1)));
    const {queue, publisher} = makePublisher(query);
    for(const sequence of [1n, 2n]) {
      publisher.pushSnapshot([
        new SequencedValue(0, new Sequence(100n)),
        new SequencedValue(20, new Sequence(100n)),
        new SequencedValue(2, new Sequence(sequence))]);
      assert.strictEqual(queue.tryPop().sequence.ordinal, sequence);
    }
    publisher.push(new SequencedValue(0, new Sequence(100n)));
    publisher.push(new SequencedValue(20, new Sequence(100n)));
    publisher.push(new SequencedValue(2, new Sequence(3n)));
    publisher.endSnapshot(1);
    assert.strictEqual(queue.tryPop().sequence.ordinal, 3n);
    publisher.push(new SequencedValue(0, new Sequence(100n)));
    publisher.push(new SequencedValue(20, new Sequence(100n)));
    publisher.push(new SequencedValue(2, new Sequence(4n)));
    assert.strictEqual(queue.tryPop().sequence.ordinal, 4n);
    assert.strictEqual(queue.tryPop(), null);
  });

  it('recovery_preserves_query_settings', () => {
    const query = new BasicQuery('original');
    const start = DateTime.fromJson('20261110T120000');
    const end = new Sequence(100n);
    query.range = new Range(start, end);
    query.snapshotLimit = SnapshotLimit.fromHead(10);
    query.interruptionPolicy = InterruptionPolicy.RECOVER_DATA;
    query.filter = ConstantExpression.FALSE;
    const original = query.clone();
    const {publisher} = makePublisher(query);
    query.index = 'changed';
    query.range = Range.EMPTY;
    query.snapshotLimit = SnapshotLimit.NONE;
    query.interruptionPolicy = InterruptionPolicy.BREAK_QUERY;
    query.filter = ConstantExpression.TRUE;
    const recovery = publisher.beginRecovery();
    assert.ok(recovery.equals(original));
    assert.notStrictEqual(recovery, original);
    recovery.index = 'changed_again';
    recovery.range = Range.EMPTY;
    recovery.snapshotLimit = SnapshotLimit.NONE;
    recovery.interruptionPolicy = InterruptionPolicy.BREAK_QUERY;
    recovery.filter = ConstantExpression.TRUE;
    assert.ok(publisher.beginRecovery().equals(original));
  });

  it('recover_data_after_last_accepted_sequence', () => {
    const query = new BasicQuery('test');
    query.range = Range.TOTAL;
    query.interruptionPolicy = InterruptionPolicy.RECOVER_DATA;
    const {queue, publisher} = makePublisher(query);
    const sequence = new Sequence(9007199254740993n);
    const first = new SequencedValue(10, sequence);
    const second = new SequencedValue(20, sequence.increment());
    const third = new SequencedValue(30, second.sequence.increment());
    publisher.endSnapshot(1);
    publisher.push(first);
    assert.deepStrictEqual(queue.tryPop(), first);
    const recovery = publisher.beginRecovery();
    assert.strictEqual(publisher.id, null);
    assert.ok(recovery.range.equals(
      new Range(sequence.increment(), Sequence.LAST)));
    assert.ok(query.range.equals(Range.TOTAL));
    publisher.push(second);
    publisher.push(third);
    publisher.pushSnapshot([first, second]);
    assert.deepStrictEqual(queue.tryPop(), second);
    assert.strictEqual(queue.tryPop(), null);
    publisher.endRecovery(2);
    assert.strictEqual(publisher.id, 2);
    assert.deepStrictEqual(queue.tryPop(), third);
    assert.strictEqual(queue.tryPop(), null);
    assert.ok(publisher.beginRecovery().range.equals(
      new Range(third.sequence.increment(), Sequence.LAST)));
  });

  it('ignore_continue_resumes_from_present', () => {
    const query = BasicQuery.makeCurrentQuery('test');
    const {queue, publisher} = makePublisher(query);
    assert.ok(publisher.beginRecovery().range.equals(Range.REAL_TIME));
    publisher.endRecovery(1);
    const previous = new SequencedValue(10, new Sequence(10n));
    publisher.push(previous);
    assert.deepStrictEqual(queue.tryPop(), previous);
    assert.ok(publisher.beginRecovery().range.equals(Range.REAL_TIME));
    const next = new SequencedValue(20, new Sequence(20n));
    publisher.push(previous);
    publisher.push(next);
    publisher.endRecovery(2);
    assert.deepStrictEqual(queue.tryPop(), next);
    assert.strictEqual(queue.tryPop(), null);
  });

  it('break_query_closes_queue', async () => {
    const {queue, publisher} = makePublisher(new BasicQuery('test'));
    publisher.endSnapshot(1);
    assert.throws(() => publisher.beginRecovery(), QueryInterruptedError);
    assert.strictEqual(publisher.id, null);
    await assert.rejects(queue.pop(), QueryInterruptedError);
  });

  it('close_and_queue_errors', async () => {
    for(const error of [undefined, new Error('Disconnected.')]) {
      const {queue, publisher} = makePublisher(
        BasicQuery.makeCurrentQuery('test'));
      publisher.endSnapshot(1);
      const pending = queue.pop();
      publisher.close(error);
      if(error) {
        await assert.rejects(pending, value => value === error);
      } else {
        await assert.rejects(pending, PipeBrokenError);
      }
      const value = new SequencedValue(10, new Sequence(1n));
      assert.throws(() => publisher.push(value));
      assert.throws(() => publisher.pushSnapshot([
        new SequencedValue(20, new Sequence(2n))]));
    }
  });
});
