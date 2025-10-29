#ifndef BEAM_STATE_PUBLISHER_HPP
#define BEAM_STATE_PUBLISHER_HPP
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Publishes the most recent update to a value.
   * @tparam T The type of data to publish.
   */
  template<typename T>
  class StatePublisher final : public SnapshotPublisher<T, T>,
      public QueueWriter<T> {
    public:
      using Type = typename SnapshotPublisher<T, T>::Type;
      using Snapshot = typename SnapshotPublisher<T, T>::Snapshot;

      /** Constructs a StatePublisher. */
      StatePublisher() = default;

      /**
       * Constructs a StatePublisher.
       * @param value The initial state of the value to publish.
       */
      template<Initializes<T> VF,
        typename = disable_copy_constructor_t<StatePublisher, VF>>
      explicit StatePublisher(VF&& value);

      void with(const std::function<void (boost::optional<const Snapshot&>)>& f)
        const override;
      void monitor(ScopedQueueWriter<Type> monitor,
        Out<boost::optional<Snapshot>> snapshot) const override;
      void with(const std::function<void ()>& f) const override;
      void monitor(ScopedQueueWriter<Type> monitor) const override;
      void push(const Type& value) override;
      void push(Type&& value) override;
      void close(const std::exception_ptr& e) override;
      using QueueWriter<T>::close;
      using SnapshotPublisher<T, T>::with;

    private:
      mutable RecursiveMutex m_mutex;
      boost::optional<Type> m_value;
      QueueWriterPublisher<Type> m_publisher;
  };

  template<typename T>
  StatePublisher(T&&) -> StatePublisher<std::remove_cvref_t<T>>;

  template<typename T>
  template<Initializes<T> VF, typename>
  StatePublisher<T>::StatePublisher(VF&& value)
    : m_value(std::forward<VF>(value)) {}

  template<typename T>
  void StatePublisher<T>::with(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    if(m_value) {
      f(*m_value);
    } else {
      f(boost::none);
    }
  }

  template<typename T>
  void StatePublisher<T>::monitor(ScopedQueueWriter<Type> queue,
      Out<boost::optional<Snapshot>> snapshot) const {
    auto lock = boost::lock_guard(m_mutex);
    *snapshot = m_value;
    m_publisher.monitor(std::move(queue));
  }

  template<typename T>
  void StatePublisher<T>::with(const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename T>
  void StatePublisher<T>::monitor(ScopedQueueWriter<Type> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    if(m_value) {
      queue.push(*m_value);
    }
    m_publisher.monitor(std::move(queue));
  }

  template<typename T>
  void StatePublisher<T>::push(const Type& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_value = value;
    m_publisher.push(*m_value);
  }

  template<typename T>
  void StatePublisher<T>::push(Type&& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_value = std::move(value);
    m_publisher.push(*m_value);
  }

  template<typename T>
  void StatePublisher<T>::close(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_value = boost::none;
    m_publisher.close(e);
  }
}

#endif
