#ifndef BEAM_VALUE_SNAPSHOT_PUBLISHER_HPP
#define BEAM_VALUE_SNAPSHOT_PUBLISHER_HPP
#include <functional>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Publishes updates to a wrapper over a value.
   * @tparam V The type of data to publish.
   * @tparam S The type of the snapshot.
   */
  template<typename V, typename S>
  class ValueSnapshotPublisher final :
      public SnapshotPublisher<V, S>, public QueueWriter<V> {
    public:
      using Type = typename SnapshotPublisher<V, S>::Type;
      using Snapshot = typename SnapshotPublisher<V, S>::Snapshot;

      /**
       * The function called to initialize a monitor.
       * @param snapshot The current Snapshot.
       * @param monitor The monitor to initialize.
       */
      using InitializationFunction = std::function<
        void (const Snapshot& snapshot, ScopedQueueWriter<Type>& monitor)>;

      /**
       * The function called to update the Snapshot.
       * @param snapshot The current Snapshot.
       * @param value The value pushed.
       * @return <code>true</code> iff the <i>value</i> should be published.
       */
      using FilteredUpdateFunction =
        std::function<bool (Snapshot& snapshot, const Type& value)>;

      /**
       * The function called to update the Snapshot.
       * @param snapshot The current Snapshot.
       * @param value The value pushed.
       */
      using UpdateFunction =
        std::function<void (Snapshot& snapshot, const Type& value)>;

      /**
       * Constructs a ValueSnapshotPublisher.
       * @param initialization The function used to initialize a monitor.
       * @param update The function used to update the Snapshot.
       * @param snapshot Initializes the Snapshot.
       */
      template<Initializes<S> SF>
      ValueSnapshotPublisher(const InitializationFunction& initialize,
        const UpdateFunction& update, SF&& snapshot);

      /**
       * Constructs a ValueSnapshotPublisher.
       * @param initialization The function used to initialize a monitor.
       * @param update The function used to update the Snapshot.
       * @param snapshot Initializes the Snapshot.
       */
      template<Initializes<S> SF>
      ValueSnapshotPublisher(
        bool filter, const InitializationFunction& initialize,
        const FilteredUpdateFunction& update, SF&& snapshot);

      void with(const std::function<void (boost::optional<const Snapshot&>)>& f)
        const override;
      void monitor(ScopedQueueWriter<Type> monitor,
        Out<boost::optional<Snapshot>> snapshot) const override;
      void with(const std::function<void ()>& f) const override;
      void monitor(ScopedQueueWriter<Type> monitor) const override;
      void push(const Type& value) override;
      void push(Type&& value) override;
      void close(const std::exception_ptr& e) override;
      using QueueWriter<V>::close;
      using SnapshotPublisher<V, S>::with;

    private:
      mutable RecursiveMutex m_mutex;
      InitializationFunction m_initialize;
      FilteredUpdateFunction m_update;
      LocalPtr<Snapshot> m_snapshot;
      QueueWriterPublisher<Type> m_publisher;
  };

  template<typename V, typename S>
  template<Initializes<S> SF>
  ValueSnapshotPublisher<V, S>::ValueSnapshotPublisher(
    const InitializationFunction& initialize, const UpdateFunction& update,
    SF&& snapshot)
    : m_initialize(initialize),
      m_update([update] (Snapshot& snapshot, const Type& value) {
        update(snapshot, value);
        return true;
      }),
      m_snapshot(std::forward<SF>(snapshot)) {}

  template<typename V, typename S>
  template<Initializes<S> SF>
  ValueSnapshotPublisher<V, S>::ValueSnapshotPublisher(
    bool filter, const InitializationFunction& initialize,
    const FilteredUpdateFunction& update, SF&& snapshot)
    : m_initialize(initialize),
      m_update(update),
      m_snapshot(std::forward<SF>(snapshot)) {}

  template<typename V, typename S>
  void ValueSnapshotPublisher<V, S>::with(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f(*m_snapshot);
  }

  template<typename V, typename S>
  void ValueSnapshotPublisher<V, S>::monitor(ScopedQueueWriter<Type> queue,
      Out<boost::optional<Snapshot>> snapshot) const {
    auto lock = boost::lock_guard(m_mutex);
    *snapshot = *m_snapshot;
    m_publisher.monitor(std::move(queue));
  }

  template<typename V, typename S>
  void ValueSnapshotPublisher<V, S>::with(
      const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename V, typename S>
  void ValueSnapshotPublisher<V, S>::monitor(
      ScopedQueueWriter<Type> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    m_initialize(*m_snapshot, queue);
    m_publisher.monitor(std::move(queue));
  }

  template<typename V, typename S>
  void ValueSnapshotPublisher<V, S>::push(const Type& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(!m_update(*m_snapshot, value)) {
      return;
    }
    m_publisher.push(value);
  }

  template<typename V, typename S>
  void ValueSnapshotPublisher<V, S>::push(Type&& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(!m_update(*m_snapshot, value)) {
      return;
    }
    m_publisher.push(std::move(value));
  }

  template<typename V, typename S>
  void ValueSnapshotPublisher<V, S>::close(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_publisher.close(e);
  }
}

#endif
