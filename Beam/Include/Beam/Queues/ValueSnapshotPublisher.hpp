#ifndef BEAM_VALUESNAPSHOTPUBLISHER_HPP
#define BEAM_VALUESNAPSHOTPUBLISHER_HPP
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /*! \class ValueSnapshotPublisher
      \brief Publishes updates to a wrapper over a value.
      \tparam ValueType The type of data to publish.
      \tparam SnapshotType The type of the snapshot.
   */
  template<typename ValueType, typename SnapshotType>
  class ValueSnapshotPublisher :
      public SnapshotPublisher<ValueType, SnapshotType>,
      public QueueWriter<ValueType> {
    public:
      using Type = typename SnapshotPublisher<ValueType, SnapshotType>::Type;
      using Snapshot = typename SnapshotPublisher<
        ValueType, SnapshotType>::Snapshot;

      //! The function called to initialize a monitor.
      /*!
        \param snapshot The current Snapshot.
        \param monitor The monitor to initialize.
      */
      using InitializationFunction = std::function<
        void (const Snapshot& snapshot,
        const std::shared_ptr<QueueWriter<Type>>& monitor)>;

      //! The function called to update the Snapshot.
      /*!
        \param snapshot The current Snapshot.
        \param value The value pushed.
        \return <code>true</code> iff the <i>value</i> should be published.
      */
      using FilteredUpdateFunction = std::function<
        bool (Snapshot& snapshot, const Type& value)>;

      //! The function called to update the Snapshot.
      /*!
        \param snapshot The current Snapshot.
        \param value The value pushed.
      */
      using UpdateFunction = std::function<
        void (Snapshot& snapshot, const Type& value)>;

      //! Constructs a ValueSnapshotPublisher.
      /*!
        \param initialization The function used to initialize a monitor.
        \param update The function used to update the Snapshot.
        \param snapshot Initializes the Snapshot.
      */
      template<typename SnapshotForward>
      ValueSnapshotPublisher(const InitializationFunction& initialize,
        const UpdateFunction& update, SnapshotForward&& snapshot);

      //! Constructs a ValueSnapshotPublisher.
      /*!
        \param initialization The function used to initialize a monitor.
        \param update The function used to update the Snapshot.
        \param snapshot Initializes the Snapshot.
      */
      template<typename SnapshotForward>
      ValueSnapshotPublisher(bool filter,
        const InitializationFunction& initialize,
        const FilteredUpdateFunction& update, SnapshotForward&& snapshot);

      virtual ~ValueSnapshotPublisher() override final;

      virtual void WithSnapshot(const std::function<
        void (boost::optional<const Snapshot&>)>& f) const override final;

      virtual void Monitor(std::shared_ptr<QueueWriter<Type>> monitor,
        Out<boost::optional<Snapshot>> snapshot) const override final;

      virtual void With(const std::function<void ()>& f) const override final;

      virtual void Monitor(
        std::shared_ptr<QueueWriter<Type>> monitor) const override final;

      virtual void Push(const Type& value) override final;

      virtual void Push(Type&& value) override final;

      virtual void Break(const std::exception_ptr& e) override final;

      using QueueWriter<ValueType>::Break;
    private:
      mutable Threading::RecursiveMutex m_mutex;
      InitializationFunction m_initialize;
      FilteredUpdateFunction m_update;
      LocalPtr<Snapshot> m_snapshot;
      MultiQueueWriter<Type> m_queue;
  };

  template<typename ValueType, typename SnapshotType>
  template<typename SnapshotForward>
  ValueSnapshotPublisher<ValueType, SnapshotType>::ValueSnapshotPublisher(
      const InitializationFunction& initialize, const UpdateFunction& update,
      SnapshotForward&& snapshot)
      : m_initialize(initialize),
        m_update(
          [update] (Snapshot& snapshot, const Type& value) {
            update(snapshot, value);
            return true;
          }),
        m_snapshot(std::forward<SnapshotForward>(snapshot)) {}

  template<typename ValueType, typename SnapshotType>
  template<typename SnapshotForward>
  ValueSnapshotPublisher<ValueType, SnapshotType>::ValueSnapshotPublisher(
      bool filter, const InitializationFunction& initialize,
      const FilteredUpdateFunction& update, SnapshotForward&& snapshot)
      : m_initialize(initialize),
        m_update(update),
        m_snapshot(std::forward<SnapshotForward>(snapshot)) {}

  template<typename ValueType, typename SnapshotType>
  ValueSnapshotPublisher<ValueType, SnapshotType>::~ValueSnapshotPublisher() {
    Break();
  }

  template<typename ValueType, typename SnapshotType>
  void ValueSnapshotPublisher<ValueType, SnapshotType>::WithSnapshot(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    f(*m_snapshot);
  }

  template<typename ValueType, typename SnapshotType>
  void ValueSnapshotPublisher<ValueType, SnapshotType>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue,
      Out<boost::optional<Snapshot>> snapshot) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    *snapshot = *m_snapshot;
    m_queue.Monitor(queue);
  }

  template<typename ValueType, typename SnapshotType>
  void ValueSnapshotPublisher<ValueType, SnapshotType>::With(
      const std::function<void ()>& f) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    f();
  }

  template<typename ValueType, typename SnapshotType>
  void ValueSnapshotPublisher<ValueType, SnapshotType>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    m_initialize(*m_snapshot, queue);
    m_queue.Monitor(queue);
  }

  template<typename ValueType, typename SnapshotType>
  void ValueSnapshotPublisher<ValueType, SnapshotType>::Push(
      const Type& value) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    if(!m_update(*m_snapshot, value)) {
      return;
    }
    m_queue.Push(value);
  }

  template<typename ValueType, typename SnapshotType>
  void ValueSnapshotPublisher<ValueType, SnapshotType>::Push(Type&& value) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    if(!m_update(*m_snapshot, value)) {
      return;
    }
    m_queue.Push(std::move(value));
  }

  template<typename ValueType, typename SnapshotType>
  void ValueSnapshotPublisher<ValueType, SnapshotType>::Break(
      const std::exception_ptr& e) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    m_queue.Break(e);
  }
}

#endif
