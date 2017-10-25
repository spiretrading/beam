#ifndef BEAM_TABLEPUBLISHER_HPP
#define BEAM_TABLEPUBLISHER_HPP
#include <unordered_map>
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /*! \class TableEntry
      \brief Represents a single entry in a table.
      \tparam KeyType The unique index/key into the table.
      \tparam ValueType The value associated with the key.
   */
  template<typename KeyType, typename ValueType>
  struct TableEntry {

    //! The unique index/key into the table.
    using Key = KeyType;

    //! The value associated with the key.
    using Value = ValueType;

    //! The unique index/key into the table.
    Key m_key;

    //! The value associated with the key.
    Value m_value;

    //! Constructs a TableEntry.
    TableEntry() = default;

    //! Constructs a TableEntry.
    /*!
      \param key The table entry's key.
      \param value The value to associate with the <i>key</i>.
    */
    TableEntry(const Key& key, const Value& value);
  };

  //! Builds a TableEntry.
  /*!
    \param key The table entry's key.
    \param value The value to associate with the <i>key</i>.
  */
  template<typename Key, typename Value>
  TableEntry<Key, Value> MakeTableEntry(const Key& key, const Value& value) {
    return TableEntry<Key, Value>(key, value);
  }

  /*! \class TablePublisher
      \brief Publishes updates to a table.
      \tparam KeyType The unique index/key into the table.
      \tparam ValueType The value associated with the key.
   */
  template<typename KeyType, typename ValueType>
  class TablePublisher : public SnapshotPublisher<
      TableEntry<KeyType, ValueType>, std::unordered_map<KeyType, ValueType>>,
      public QueueWriter<TableEntry<KeyType, ValueType>> {
    public:
      using Type = typename Publisher<TableEntry<KeyType, ValueType>>::Type;
      using Snapshot = typename SnapshotPublisher<TableEntry<
        KeyType, ValueType>, std::unordered_map<KeyType, ValueType>>::Snapshot;

      //! The unique index/key into the table.
      using Key = KeyType;

      //! The value associated with the key.
      using Value = ValueType;

      //! Constructs a TablePublisher.
      TablePublisher() = default;

      virtual ~TablePublisher() override final;

      //! Pushes a key/value pair onto the table.
      /*!
        \param key The table entry's key.
        \param value The value to associate with the <i>key</i>.
      */
      void Push(const Key& key, const Value& value);

      //! Deletes a key/value pair from the table.
      /*!
        \param key The key to delete.
        \param value The value to publish indicating the value is being deleted.
      */
      void Delete(const Key& key, const Value& value);

      //! Deletes a key/value pair from the table.
      /*!
        \param key The key to delete.
        \param value The value to publish indicating the value is being deleted.
      */
      void Delete(const Type& value);

      virtual void WithSnapshot(const std::function<
        void (boost::optional<const Snapshot&>)>& f) const override final;

      virtual void Monitor(std::shared_ptr<QueueWriter<Type>> queue,
        Out<boost::optional<Snapshot>> snapshot) const override final;

      virtual void With(const std::function<void ()>& f) const override final;

      virtual void Monitor(
        std::shared_ptr<QueueWriter<Type>> monitor) const override final;

      virtual void Push(const Type& value) override final;

      virtual void Push(Type&& value) override final;

      virtual void Break(const std::exception_ptr& e) override final;

      using QueueWriter<TableEntry<KeyType, ValueType>>::Break;
    private:
      mutable Threading::RecursiveMutex m_mutex;
      std::unordered_map<Key, Value> m_table;
      MultiQueueWriter<Type> m_queue;
  };

  template<typename KeyType, typename ValueType>
  TableEntry<KeyType, ValueType>::TableEntry(const Key& key, const Value& value)
      : m_key(key),
        m_value(value) {}

  template<typename KeyType, typename ValueType>
  TablePublisher<KeyType, ValueType>::~TablePublisher() {
    Break();
  }

  template<typename KeyType, typename ValueType>
  void TablePublisher<KeyType, ValueType>::Push(const Key& key,
      const Value& value) {
    Push(Type(key, value));
  }

  template<typename KeyType, typename ValueType>
  void TablePublisher<KeyType, ValueType>::Delete(const Key& key,
      const Value& value) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    m_table.erase(key);
    m_queue.Push(Type(key, value));
  }

  template<typename KeyType, typename ValueType>
  void TablePublisher<KeyType, ValueType>::Delete(const Type& value) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    m_table.erase(value.m_key);
    m_queue.Push(value);
  }

  template<typename KeyType, typename ValueType>
  void TablePublisher<KeyType, ValueType>::WithSnapshot(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    f(m_table);
  }

  template<typename KeyType, typename ValueType>
  void TablePublisher<KeyType, ValueType>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue,
      Out<boost::optional<Snapshot>> snapshot) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    *snapshot = m_table;
    m_queue.Monitor(queue);
  }

  template<typename KeyType, typename ValueType>
  void TablePublisher<KeyType, ValueType>::With(
      const std::function<void ()>& f) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    f();
  }

  template<typename KeyType, typename ValueType>
  void TablePublisher<KeyType, ValueType>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    for(auto& i : m_table) {
      queue->Push(Type(i.first, i.second));
    }
    m_queue.Monitor(queue);
  }

  template<typename KeyType, typename ValueType>
  void TablePublisher<KeyType, ValueType>::Push(const Type& value) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    m_table[value.m_key] = value.m_value;
    m_queue.Push(value);
  }

  template<typename KeyType, typename ValueType>
  void TablePublisher<KeyType, ValueType>::Push(Type&& value) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    m_table[value.m_key] = value.m_value;
    m_queue.Push(std::move(value));
  }

  template<typename KeyType, typename ValueType>
  void TablePublisher<KeyType, ValueType>::Break(const std::exception_ptr& e) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    m_queue.Break(e);
  }
}

namespace Beam {
namespace Serialization {
  template<typename KeyType, typename ValueType>
  struct Shuttle<TableEntry<KeyType, ValueType>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, TableEntry<KeyType, ValueType>& value,
        unsigned int version) const {
      shuttle.Shuttle("key", value.m_key);
      shuttle.Shuttle("value", value.m_value);
    }
  };
}
}

#endif
