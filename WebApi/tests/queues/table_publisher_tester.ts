import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import { CallbackQueueWriter, DirectoryEntry, KeyValuePair, Map,
  PipeBrokenError, Queue, SnapshotPublisher, TablePublisher } from
  '../../source';

describe('TablePublisher', () => {
  it('push_updates_table_and_monitors', () => {
    const publisher = new TablePublisher<number, string>();
    const first = new Queue<KeyValuePair<number, string>>();
    const second = new Queue<KeyValuePair<number, string>>();
    publisher.monitor(first);
    publisher.monitor(second);
    publisher.push(1, 'one');
    publisher.push(new KeyValuePair(1, 'updated'));
    for(const queue of [first, second]) {
      assert.ok(queue.tryPop().equals(new KeyValuePair(1, 'one')));
      assert.ok(queue.tryPop().equals(new KeyValuePair(1, 'updated')));
      assert.equal(queue.tryPop(), null);
    }
    assert.equal(publisher.getSnapshot().size, 1);
    assert.equal(publisher.getSnapshot().get(1), 'updated');
  });

  it('monitor_replays_table_or_captures_separate_snapshot', () => {
    const table = new TablePublisher<number, string>();
    const publisher: SnapshotPublisher<KeyValuePair<number, string>,
      Map<number, string>> = table;
    assert.equal(publisher.getSnapshot().size, 0);
    table.push(1, 'one');
    table.push(2, 'two');
    const replay = new Queue<KeyValuePair<number, string>>();
    publisher.monitor(replay);
    const entries = [replay.tryPop(), replay.tryPop()];
    assert.ok(entries.some(entry => entry.equals(new KeyValuePair(1, 'one'))));
    assert.ok(entries.some(entry => entry.equals(new KeyValuePair(2, 'two'))));
    assert.equal(replay.tryPop(), null);
    const updates = new Queue<KeyValuePair<number, string>>();
    const snapshot: {value: Map<number, string>} = {value: null};
    publisher.monitor(updates, snapshot);
    assert.equal(snapshot.value.size, 2);
    assert.equal(snapshot.value.get(1), 'one');
    assert.equal(snapshot.value.get(2), 'two');
    assert.equal(updates.tryPop(), null);
    table.push(1, 'updated');
    for(const queue of [replay, updates]) {
      assert.ok(queue.tryPop().equals(new KeyValuePair(1, 'updated')));
    }
    assert.equal(snapshot.value.get(1), 'one');
  });

  it('snapshot_copies', () => {
    const publisher = new TablePublisher<number, {value: string}>();
    const value = {value: 'one'};
    publisher.push(1, value);
    const snapshot = publisher.getSnapshot();
    assert.equal(snapshot.get(1), value);
    snapshot.delete(1);
    snapshot.set(2, {value: 'two'});
    assert.equal(publisher.getSnapshot().size, 1);
    assert.equal(publisher.getSnapshot().get(1), value);
    assert.equal(publisher.getSnapshot().get(2), undefined);
  });

  it('erase_publishes_previous_or_custom_value', () => {
    const publisher = new TablePublisher<number, string>();
    publisher.push(1, 'one');
    publisher.push(2, 'two');
    publisher.push(3, 'three');
    const queue = new Queue<KeyValuePair<number, string>>();
    const snapshot: {value: Map<number, string>} = {value: null};
    publisher.monitor(queue, snapshot);
    publisher.erase(1);
    publisher.erase(2, 'deleted');
    publisher.erase(99);
    publisher.erase(99, 'missing');
    assert.ok(queue.tryPop().equals(new KeyValuePair(1, 'one')));
    assert.ok(queue.tryPop().equals(new KeyValuePair(2, 'deleted')));
    assert.equal(queue.tryPop(), null);
    assert.equal(publisher.getSnapshot().size, 1);
    assert.equal(publisher.getSnapshot().get(3), 'three');
    assert.equal(snapshot.value.size, 3);
  });

  it('equivalent_keys_share_one_entry', () => {
    const publisher = new TablePublisher<DirectoryEntry, number>();
    const first = DirectoryEntry.makeAccount(7, 'account');
    const equivalent = DirectoryEntry.makeAccount(7, 'account');
    publisher.push(first, 10);
    publisher.push(equivalent, 20);
    assert.equal(publisher.getSnapshot().size, 1);
    assert.equal(publisher.getSnapshot().get(first), 20);
    publisher.erase(equivalent);
    assert.equal(publisher.getSnapshot().size, 0);
  });

  it('observers_see_updated_snapshot', () => {
    const publisher = new TablePublisher<number, string>();
    const snapshots: Map<number, string>[] = [];
    publisher.monitor(new CallbackQueueWriter(() => {
      snapshots.push(publisher.getSnapshot());
    }));
    publisher.push(1, 'one');
    publisher.erase(1);
    assert.equal(snapshots[0].get(1), 'one');
    assert.equal(snapshots[1].size, 0);
  });

  it('failed_monitor_does_not_stop_other_monitors', () => {
    const publisher = new TablePublisher<number, string>();
    let calls = 0;
    publisher.monitor(new CallbackQueueWriter(() => {
      ++calls;
      throw new Error('Monitor failed.');
    }));
    const queue = new Queue<KeyValuePair<number, string>>();
    publisher.monitor(queue);
    publisher.push(1, 'one');
    publisher.push(2, 'two');
    assert.equal(calls, 1);
    assert.ok(queue.tryPop().equals(new KeyValuePair(1, 'one')));
    assert.ok(queue.tryPop().equals(new KeyValuePair(2, 'two')));
    assert.equal(publisher.getSnapshot().size, 2);
  });

  it('close_retains_snapshot_and_closes_late_monitors', async () => {
    const publisher = new TablePublisher<number, string>();
    const queue = new Queue<KeyValuePair<number, string>>();
    publisher.monitor(queue);
    publisher.push(1, 'one');
    const error = new Error('Closed.');
    publisher.close(error);
    publisher.close(new Error('Ignored.'));
    assert.ok(queue.tryPop().equals(new KeyValuePair(1, 'one')));
    await assert.rejects(queue.pop(), value => value === error);
    assert.equal(publisher.getSnapshot().get(1), 'one');
    const replay = new Queue<KeyValuePair<number, string>>();
    publisher.monitor(replay);
    assert.ok(replay.tryPop().equals(new KeyValuePair(1, 'one')));
    await assert.rejects(replay.pop(), value => value === error);
    const updates = new Queue<KeyValuePair<number, string>>();
    const snapshot: {value: Map<number, string>} = {value: null};
    publisher.monitor(updates, snapshot);
    assert.equal(snapshot.value.get(1), 'one');
    await assert.rejects(updates.pop(), value => value === error);
  });

  it('default_close_and_push_after_close', async () => {
    const publisher = new TablePublisher<number, string>();
    const queue = new Queue<KeyValuePair<number, string>>();
    publisher.monitor(queue);
    publisher.close();
    await assert.rejects(queue.pop(), PipeBrokenError);
    assert.throws(() => publisher.push(1, 'one'), PipeBrokenError);
  });
});
