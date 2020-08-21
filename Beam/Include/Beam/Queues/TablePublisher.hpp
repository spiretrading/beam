#ifndef BEAM_TABLE_PUBLISHER_HPP
#define BEAM_TABLE_PUBLISHER_HPP
#include <unordered_map>
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Utilities/KeyValuePair.hpp"

namespace Beam {

  /**
   * Publishes updates to a table.
   * @param <K> The unique index/key into the table.
   * @param <V> The value associated with the key.
   */
  template<typename K, typename V>
  class TablePublisher final :
      public SnapshotPublisher<KeyValuePair<K, V>, std::unordered_map<K, V>>,
      public QueueWriter<KeyValuePair<K, V>> {
    public:
      using Type = typename Publisher<KeyValuePair<K, V>>::Type;
      using Snapshot = typename SnapshotPublisher<KeyValuePair<K, V>,
        std::unordered_map<K, V>>::Snapshot;

      /** The unique index/key into the table. */
      using Key = K;

      /** The value associated with the key. */
      using Value = V;

      /** Constructs a TablePublisher. */
      TablePublisher() = default;

      /**
       * Pushes a key/value pair onto the table.
       * @param key The table entry's key.
       * @param value The value to associate with the <i>key</i>.
       */
      void Push(const Key& key, const Value& value);

      /**
       * Deletes a key/value pair from the table.
       * @param key The key to delete.
       * @param value The value to publish indicating the value is being
       *        deleted.
       */
      void Delete(const Key& key, const Value& value);

      /**
       * Deletes a key/value pair from the table.
       * @param key The key to delete.
       * @param value The value to publish indicating the value is being
       *        deleted.
       */
      void Delete(const Type& value);

      void With(
        const std::function<void (boost::optional<const Snapshot&>)>& f)
        const override;

      void Monitor(ScopedQueueWriter<Type> queue,
        Out<boost::optional<Snapshot>> snapshot) const override;

      void With(const std::function<void ()>& f) const override;

      void Monitor(ScopedQueueWriter<Type> monitor) const override;

      void Push(const Type& value) override;

      void Push(Type&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<KeyValuePair<Key, Value>>::Break;
      using SnapshotPublisher<
        KeyValuePair<K, V>, std::unordered_map<K, V>>::With;
    private:
      mutable Threading::RecursiveMutex m_mutex;
      std::unordered_map<Key, Value> m_table;
      QueueWriterPublisher<Type> m_publisher;
  };

  template<typename K, typename V>
  void TablePublisher<K, V>::Push(const Key& key, const Value& value) {
    Push(Type{key, value});
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::Delete(const Key& key, const Value& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_table.erase(key);
    m_publisher.Push(Type{key, value});
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::Delete(const Type& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_table.erase(value.m_key);
    m_publisher.Push(value);
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::With(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f(m_table);
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::Monitor(ScopedQueueWriter<Type> queue,
      Out<boost::optional<Snapshot>> snapshot) const {
    auto lock = boost::lock_guard(m_mutex);
    *snapshot = m_table;
    m_publisher.Monitor(std::move(queue));
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::With(const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::Monitor(ScopedQueueWriter<Type> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    for(auto& i : m_table) {
      queue.Push(Type{i.first, i.second});
    }
    m_publisher.Monitor(std::move(queue));
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::Push(const Type& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_table[value.m_key] = value.m_value;
    m_publisher.Push(value);
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::Push(Type&& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_table[value.m_key] = value.m_value;
    m_publisher.Push(std::move(value));
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::Break(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_publisher.Break(e);
  }
}

#endif
