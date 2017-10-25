#ifndef BEAM_MULTIUPDATETABLEPUBLISHER_HPP
#define BEAM_MULTIUPDATETABLEPUBLISHER_HPP
#include <unordered_map>
#include <vector>
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/TablePublisher.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /*! \class MultiUpdateTablePublisher
      \brief Publishes multiple updates to a table.
      \tparam KeyType The unique index/key into the table.
      \tparam ValueType The value associated with the key.
   */
  template<typename KeyType, typename ValueType>
  class MultiUpdateTablePublisher : public SnapshotPublisher<
      std::vector<TableEntry<KeyType, ValueType>>,
      std::unordered_map<KeyType, ValueType>>,
      public QueueWriter<std::vector<TableEntry<KeyType, ValueType>>> {
    public:
      using Type = typename SnapshotPublisher<
        std::vector<TableEntry<KeyType, ValueType>>,
        std::unordered_map<KeyType, ValueType>>::Type;
      using Snapshot = typename SnapshotPublisher<
        std::vector<TableEntry<KeyType, ValueType>>,
        std::unordered_map<KeyType, ValueType>>::Snapshot;

      //! The unique index/key into the table.
      using Key = KeyType;

      //! The value associated with the key.
      using Value = ValueType;

      //! Constructs a MultiUpdateTablePublisher.
      MultiUpdateTablePublisher() = default;

      virtual ~MultiUpdateTablePublisher() override final;

      //! Gives synchronized access to the snapshot.
      /*!
        \param f The action to perform on the snapshot.
      */
      template<typename F>
      void WithSnapshot(F f);

      //! Pushes a single update.
      /*!
        \param update The update to push.
      */
      void Push(const TableEntry<Key, Value>& update);

      //! Pushes a key/value pair onto the table.
      /*!
        \param key The table entry's key.
        \param value The value to associate with the <i>key</i>.
      */
      void Push(const Key& key, const Value& value);

      virtual void WithSnapshot(
        const std::function<void (boost::optional<const Snapshot&>)>& f)
        const = 0;

      virtual void Monitor(std::shared_ptr<QueueWriter<Type>> monitor,
        Out<boost::optional<Snapshot>> snapshot) const override final;

      virtual void With(const std::function<void ()>& f) const override final;

      virtual void Monitor(
        std::shared_ptr<QueueWriter<Type>> monitor) const override final;

      virtual void Push(const Type& value) override final;

      virtual void Break(const std::exception_ptr& e) override final;
    private:
      mutable Threading::RecursiveMutex m_mutex;
      std::unordered_map<Key, Value> m_table;
      MultiQueueWriter<Type> m_queue;
  };

  template<typename KeyType, typename ValueType>
  virtual MultiUpdateTablePublisher<KeyType, ValueType>::
      ~MultiUpdateTablePublisher() {
    Break();
  }

  template<typename KeyType, typename ValueType>
  template<typename F>
  void MultiUpdateTablePublisher<KeyType, ValueType>::WithSnapshot(F f) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    f(m_table);
  }

  template<typename KeyType, typename ValueType>
  void MultiUpdateTablePublisher<KeyType, ValueType>::Push(
      const TableEntry<Key, Value>& update) {
    std::vector<TableEntry<Key, Value>> v;
    v.push_back(update);
    Push(v);
  }

  template<typename KeyType, typename ValueType>
  void MultiUpdateTablePublisher<KeyType, ValueType>::Push(const Key& key,
      const Value& value) {
    Push(MakeTableEntry(key, value));
  }

  template<typename KeyType, typename ValueType>
  void MultiUpdateTablePublisher<KeyType, ValueType>::WithSnapshot(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    f(m_table);
  }

  template<typename KeyType, typename ValueType>
  void MultiUpdateTablePublisher<KeyType, ValueType>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue,
      Out<boost::optional<Snapshot>> snapshot) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    *snapshot = m_table;
    m_queue.Monitor(queue);
  }

  template<typename KeyType, typename ValueType>
  void MultiUpdateTablePublisher<KeyType, ValueType>::With(
      const std::function<void ()>& f) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    f();
  }

  template<typename KeyType, typename ValueType>
  void MultiUpdateTablePublisher<KeyType, ValueType>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    if(!m_table.empty()) {
      Type update;
      for(auto& i : m_table) {
        update.push_back(TableEntry<KeyType, ValueType>(i.first, i.second));
      }
      queue->Push(update);
    }
    m_queue.Monitor(queue);
  }

  template<typename KeyType, typename ValueType>
  void MultiUpdateTablePublisher<KeyType, ValueType>::Push(const Type& value) {
    if(value.empty()) {
      return;
    }
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    for(auto& i : value) {
      m_table[i.m_key] = i.m_value;
    }
    m_queue.Push(value);
  }

  template<typename KeyType, typename ValueType>
  void MultiUpdateTablePublisher<KeyType, ValueType>::Break(
      const std::exception_ptr& e) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    m_queue.Break(e);
  }
}

#endif
