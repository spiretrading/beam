#ifndef BEAM_TABLE_PUBLISHER_HPP
#define BEAM_TABLE_PUBLISHER_HPP
#include <unordered_map>
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Utilities/KeyValuePair.hpp"

namespace Beam {

  /**
   * Publishes updates to a table.
   * @tparam K The unique index/key into the table.
   * @tparam V The value associated with the key.
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
      void push(const Key& key, const Value& value);

      /**
       * Removes a key/value pair from the table.
       * @param key The key to remove.
       */
      void erase(const Key& key);

      void with(const std::function<void (boost::optional<const Snapshot&>)>& f)
        const override;
      void monitor(ScopedQueueWriter<Type> queue,
        Out<boost::optional<Snapshot>> snapshot) const override;
      void with(const std::function<void ()>& f) const override;
      void monitor(ScopedQueueWriter<Type> monitor) const override;
      void push(const Type& value) override;
      void push(Type&& value) override;
      void close(const std::exception_ptr& e) override;
      using QueueWriter<KeyValuePair<Key, Value>>::close;
      using SnapshotPublisher<
        KeyValuePair<K, V>, std::unordered_map<K, V>>::with;

    private:
      mutable RecursiveMutex m_mutex;
      std::unordered_map<Key, Value> m_table;
      QueueWriterPublisher<Type> m_publisher;
  };

  template<typename K, typename V>
  void TablePublisher<K, V>::push(const Key& key, const Value& value) {
    push(Type(key, value));
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::erase(const Key& key) {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_table.find(key);
    if(i == m_table.end()) {
      return;
    }
    auto value = std::move(i->second);
    m_table.erase(i);
    m_publisher.push(Type(key, value));
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::with(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f(m_table);
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::monitor(ScopedQueueWriter<Type> queue,
      Out<boost::optional<Snapshot>> snapshot) const {
    auto lock = boost::lock_guard(m_mutex);
    *snapshot = m_table;
    m_publisher.monitor(std::move(queue));
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::with(const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::monitor(ScopedQueueWriter<Type> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    for(auto& i : m_table) {
      queue.push(Type(i.first, i.second));
    }
    m_publisher.monitor(std::move(queue));
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::push(const Type& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_table[value.m_key] = value.m_value;
    m_publisher.push(value);
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::push(Type&& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_table[value.m_key] = value.m_value;
    m_publisher.push(std::move(value));
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::close(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_publisher.close(e);
  }
}

#endif
