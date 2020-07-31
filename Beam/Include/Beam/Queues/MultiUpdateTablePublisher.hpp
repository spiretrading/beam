#ifndef BEAM_MULTI_UPDATE_TABLE_PUBLISHER_HPP
#define BEAM_MULTI_UPDATE_TABLE_PUBLISHER_HPP
#include <unordered_map>
#include <vector>
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/TablePublisher.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /**
   * Publishes multiple updates to a table.
   * @param <K> The unique index/key into the table.
   * @param <V> The value associated with the key.
   */
  template<typename K, typename V>
  class MultiUpdateTablePublisher final : public SnapshotPublisher<
      std::vector<TableEntry<K, V>>, std::unordered_map<K, V>>,
      public QueueWriter<std::vector<TableEntry<K, V>>> {
    public:
      using Type = typename SnapshotPublisher<std::vector<TableEntry<K, V>>,
        std::unordered_map<K, V>>::Type;
      using Snapshot = typename SnapshotPublisher<std::vector<TableEntry<K, V>>,
        std::unordered_map<K, V>>::Snapshot;

      /** The unique index/key into the table. */
      using Key = K;

      /** The value associated with the key. */
      using Value = V;

      /** Constructs a MultiUpdateTablePublisher. */
      MultiUpdateTablePublisher() = default;

      /**
       * Gives synchronized access to the snapshot.
       * @param f The action to perform on the snapshot.
       */
      template<typename F>
      void WithSnapshot(F f);

      /**
       * Pushes a single update.
       * @param update The update to push.
       */
      void Push(const TableEntry<Key, Value>& update);

      /**
       * Pushes a key/value pair onto the table.
       * @param key The table entry's key.
       * @param value The value to associate with the <i>key</i>.
       */
      void Push(const Key& key, const Value& value);

      virtual void WithSnapshot(
        const std::function<void (boost::optional<const Snapshot&>)>& f)
        const = 0;

      void Monitor(std::shared_ptr<QueueWriter<Type>> monitor,
        Out<boost::optional<Snapshot>> snapshot) const override;

      void With(const std::function<void ()>& f) const override;

      void Monitor(std::shared_ptr<QueueWriter<Type>> monitor) const override;

      void Push(const Type& value) override;

      void Break(const std::exception_ptr& e) override;

    private:
      mutable Threading::RecursiveMutex m_mutex;
      std::unordered_map<Key, Value> m_table;
      MultiQueueWriter<Type> m_queue;
  };

  template<typename K, typename V>
  template<typename F>
  void MultiUpdateTablePublisher<K, V>::WithSnapshot(F f) {
    auto lock = boost::lock_guard(m_mutex);
    f(m_table);
  }

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::Push(
      const TableEntry<Key, Value>& update) {
    auto value = std::vector<TableEntry<Key, Value>>();
    value.push_back(update);
    Push(value);
  }

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::Push(const Key& key,
      const Value& value) {
    Push(TableEntry{key, value});
  }

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::WithSnapshot(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f(m_table);
  }

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue,
      Out<boost::optional<Snapshot>> snapshot) const {
    auto lock = boost::lock_guard(m_mutex);
    *snapshot = m_table;
    m_queue.Monitor(queue);
  }

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::With(
      const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    if(!m_table.empty()) {
      auto update = Type();
      for(auto& i : m_table) {
        update.push_back(TableEntry{i.first, i.second});
      }
      queue->Push(std::move(update));
    }
    m_queue.Monitor(queue);
  }

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::Push(const Type& value) {
    if(value.empty()) {
      return;
    }
    auto lock = boost::lock_guard(m_mutex);
    for(auto& i : value) {
      m_table[i.m_key] = i.m_value;
    }
    m_queue.Push(value);
  }

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::Break(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_queue.Break(e);
  }
}

#endif
