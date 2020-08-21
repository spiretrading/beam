#ifndef BEAM_VALUE_SNAPSHOT_PUBLISHER_HPP
#define BEAM_VALUE_SNAPSHOT_PUBLISHER_HPP
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /**
   * Publishes updates to a wrapper over a value.
   * @param <V> The type of data to publish.
   * @param <S> The type of the snapshot.
   */
  template<typename V, typename S>
  class ValueSnapshotPublisher final : public SnapshotPublisher<V, S>,
      public QueueWriter<V> {
    public:
      using Type = typename SnapshotPublisher<V, S>::Type;
      using Snapshot = typename SnapshotPublisher<V, S>::Snapshot;

      /**
       * The function called to initialize a monitor.
       * @param snapshot The current Snapshot.
       * @param monitor The monitor to initialize.
       */
      using InitializationFunction = std::function<void (
        const Snapshot& snapshot, ScopedQueueWriter<Type>& monitor)>;

      /**
       * The function called to update the Snapshot.
       * @param snapshot The current Snapshot.
       * @param value The value pushed.
       * @return <code>true</code> iff the <i>value</i> should be published.
       */
      using FilteredUpdateFunction = std::function<bool (
        Snapshot& snapshot, const Type& value)>;

      /**
       * The function called to update the Snapshot.
       * @param snapshot The current Snapshot.
       * @param value The value pushed.
       */
      using UpdateFunction = std::function<void (
        Snapshot& snapshot, const Type& value)>;

      /**
       * Constructs a ValueSnapshotPublisher.
       * @param initialization The function used to initialize a monitor.
       * @param update The function used to update the Snapshot.
       * @param snapshot Initializes the Snapshot.
       */
      template<typename SF>
      ValueSnapshotPublisher(const InitializationFunction& initialize,
        const UpdateFunction& update, SF&& snapshot);

      /**
       * Constructs a ValueSnapshotPublisher.
       * @param initialization The function used to initialize a monitor.
       * @param update The function used to update the Snapshot.
       * @param snapshot Initializes the Snapshot.
       */
      template<typename SF>
      ValueSnapshotPublisher(bool filter,
        const InitializationFunction& initialize,
        const FilteredUpdateFunction& update, SF&& snapshot);

      void With(
        const std::function<void (boost::optional<const Snapshot&>)>& f)
        const override;

      void Monitor(ScopedQueueWriter<Type> monitor,
        Out<boost::optional<Snapshot>> snapshot) const override;

      void With(const std::function<void ()>& f) const override;

      void Monitor(ScopedQueueWriter<Type> monitor) const override;

      void Push(const Type& value) override;

      void Push(Type&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<V>::Break;
      using SnapshotPublisher<V, S>::With;
    private:
      mutable Threading::RecursiveMutex m_mutex;
      InitializationFunction m_initialize;
      FilteredUpdateFunction m_update;
      LocalPtr<Snapshot> m_snapshot;
      QueueWriterPublisher<Type> m_publisher;
  };

  template<typename V, typename S>
  template<typename SF>
  ValueSnapshotPublisher<V, S>::ValueSnapshotPublisher(
    const InitializationFunction& initialize, const UpdateFunction& update,
    SF&& snapshot)
    : m_initialize(initialize),
      m_update(
        [update] (Snapshot& snapshot, const Type& value) {
          update(snapshot, value);
          return true;
        }),
      m_snapshot(std::forward<SF>(snapshot)) {}

  template<typename V, typename S>
  template<typename SF>
  ValueSnapshotPublisher<V, S>::ValueSnapshotPublisher(bool filter,
    const InitializationFunction& initialize,
    const FilteredUpdateFunction& update, SF&& snapshot)
    : m_initialize(initialize),
      m_update(update),
      m_snapshot(std::forward<SF>(snapshot)) {}

  template<typename V, typename S>
  void ValueSnapshotPublisher<V, S>::With(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f(*m_snapshot);
  }

  template<typename V, typename S>
  void ValueSnapshotPublisher<V, S>::Monitor(ScopedQueueWriter<Type> queue,
      Out<boost::optional<Snapshot>> snapshot) const {
    auto lock = boost::lock_guard(m_mutex);
    *snapshot = *m_snapshot;
    m_publisher.Monitor(std::move(queue));
  }

  template<typename V, typename S>
  void ValueSnapshotPublisher<V, S>::With(
      const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename V, typename S>
  void ValueSnapshotPublisher<V, S>::Monitor(
      ScopedQueueWriter<Type> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    m_initialize(*m_snapshot, queue);
    m_publisher.Monitor(std::move(queue));
  }

  template<typename V, typename S>
  void ValueSnapshotPublisher<V, S>::Push(const Type& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(!m_update(*m_snapshot, value)) {
      return;
    }
    m_publisher.Push(value);
  }

  template<typename V, typename S>
  void ValueSnapshotPublisher<V, S>::Push(Type&& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(!m_update(*m_snapshot, value)) {
      return;
    }
    m_publisher.Push(std::move(value));
  }

  template<typename V, typename S>
  void ValueSnapshotPublisher<V, S>::Break(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_publisher.Break(e);
  }
}

#endif
