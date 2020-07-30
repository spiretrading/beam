#ifndef BEAM_TABLE_PUBLISHER_HPP
#define BEAM_TABLE_PUBLISHER_HPP
#include <unordered_map>
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /**
   * Represents a single entry in a table.
   * @param <K> The unique index/key into the table.
   * @param <V> The value associated with the key.
   */
  template<typename K, typename V>
  struct TableEntry {

    /** The unique index/key into the table. */
    using Key = K;

    /** The value associated with the key. */
    using Value = V;

    /** The unique index/key into the table. */
    Key m_key;

    /** The value associated with the key. */
    Value m_value;
  };

  /**
   * Publishes updates to a table.
   * @param <K> The unique index/key into the table.
   * @param <V> The value associated with the key.
   */
  template<typename K, typename V>
  class TablePublisher final :
      public SnapshotPublisher<TableEntry<K, V>, std::unordered_map<K, V>>,
      public QueueWriter<TableEntry<K, V>> {
    public:
      using Type = typename Publisher<TableEntry<K, V>>::Type;
      using Snapshot = typename SnapshotPublisher<TableEntry<K, V>,
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

      void WithSnapshot(const std::function<
        void (boost::optional<const Snapshot&>)>& f) const override;

      void Monitor(std::shared_ptr<QueueWriter<Type>> queue,
        Out<boost::optional<Snapshot>> snapshot) const override;

      void With(const std::function<void ()>& f) const override;

      void Monitor(std::shared_ptr<QueueWriter<Type>> monitor) const override;

      void Push(const Type& value) override;

      void Push(Type&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<TableEntry<Key, Value>>::Break;
    private:
      mutable Threading::RecursiveMutex m_mutex;
      std::unordered_map<Key, Value> m_table;
      MultiQueueWriter<Type> m_queue;
  };

  template<typename K, typename V>
  void TablePublisher<K, V>::Push(const Key& key, const Value& value) {
    Push(Type{key, value});
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::Delete(const Key& key, const Value& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_table.erase(key);
    m_queue.Push(Type{key, value});
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::Delete(const Type& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_table.erase(value.m_key);
    m_queue.Push(value);
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::WithSnapshot(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f(m_table);
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::Monitor(std::shared_ptr<QueueWriter<Type>> queue,
      Out<boost::optional<Snapshot>> snapshot) const {
    auto lock = boost::lock_guard(m_mutex);
    *snapshot = m_table;
    m_queue.Monitor(queue);
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::With(const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    for(auto& i : m_table) {
      queue->Push(Type{i.first, i.second});
    }
    m_queue.Monitor(queue);
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::Push(const Type& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_table[value.m_key] = value.m_value;
    m_queue.Push(value);
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::Push(Type&& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_table[value.m_key] = value.m_value;
    m_queue.Push(std::move(value));
  }

  template<typename K, typename V>
  void TablePublisher<K, V>::Break(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_queue.Break(e);
  }
}

namespace Beam::Serialization {
  template<typename K, typename V>
  struct Shuttle<TableEntry<K, V>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, TableEntry<K, V>& value,
        unsigned int version) const {
      shuttle.Shuttle("key", value.m_key);
      shuttle.Shuttle("value", value.m_value);
    }
  };
}

#endif
