#ifndef BEAM_MULTI_UPDATE_TABLE_PUBLISHER_HPP
#define BEAM_MULTI_UPDATE_TABLE_PUBLISHER_HPP
#include <unordered_map>
#include <vector>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Utilities/KeyValuePair.hpp"

namespace Beam {

  /**
   * Publishes multiple updates to a table.
   * @param <K> The unique index/key into the table.
   * @param <V> The value associated with the key.
   */
  template<typename K, typename V>
  class MultiUpdateTablePublisher final : public SnapshotPublisher<
      std::vector<KeyValuePair<K, V>>, std::unordered_map<K, V>>,
      public QueueWriter<std::vector<KeyValuePair<K, V>>> {
    public:
      using Type = typename SnapshotPublisher<std::vector<KeyValuePair<K, V>>,
        std::unordered_map<K, V>>::Type;
      using Snapshot = typename SnapshotPublisher<
        std::vector<KeyValuePair<K, V>>, std::unordered_map<K, V>>::Snapshot;

      /** The unique index/key into the table. */
      using Key = K;

      /** The value associated with the key. */
      using Value = V;

      /** Constructs a MultiUpdateTablePublisher. */
      MultiUpdateTablePublisher() = default;

      /**
       * Pushes a single update.
       * @param update The update to push.
       */
      void Push(const KeyValuePair<Key, Value>& update);

      /**
       * Pushes a key/value pair onto the table.
       * @param key The table entry's key.
       * @param value The value to associate with the <i>key</i>.
       */
      void Push(const Key& key, const Value& value);

      void With(
        const std::function<void (boost::optional<const Snapshot&>)>& f)
        const override;

      void Monitor(ScopedQueueWriter<Type> monitor,
        Out<boost::optional<Snapshot>> snapshot) const override;

      void With(const std::function<void ()>& f) const override;

      void Monitor(ScopedQueueWriter<Type> monitor) const override;

      void Push(const Type& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<std::vector<KeyValuePair<K, V>>>::Break;
      using SnapshotPublisher<
        std::vector<KeyValuePair<K, V>>, std::unordered_map<K, V>>::With;
    private:
      mutable Threading::RecursiveMutex m_mutex;
      std::unordered_map<Key, Value> m_table;
      QueueWriterPublisher<Type> m_publisher;
  };

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::Push(
      const KeyValuePair<Key, Value>& update) {
    auto value = std::vector<KeyValuePair<Key, Value>>();
    value.push_back(update);
    Push(value);
  }

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::Push(const Key& key,
      const Value& value) {
    Push(KeyValuePair(key, value));
  }

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::With(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f(m_table);
  }

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::Monitor(ScopedQueueWriter<Type> queue,
      Out<boost::optional<Snapshot>> snapshot) const {
    auto lock = boost::lock_guard(m_mutex);
    *snapshot = m_table;
    m_publisher.Monitor(std::move(queue));
  }

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::With(
      const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::Monitor(
      ScopedQueueWriter<Type> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    if(!m_table.empty()) {
      auto update = Type();
      for(auto& i : m_table) {
        update.push_back(KeyValuePair(i.first, i.second));
      }
      queue.Push(std::move(update));
    }
    m_publisher.Monitor(std::move(queue));
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
    m_publisher.Push(value);
  }

  template<typename K, typename V>
  void MultiUpdateTablePublisher<K, V>::Break(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_publisher.Break(e);
  }
}

#endif
