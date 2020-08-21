#ifndef BEAM_STATE_PUBLISHER_HPP
#define BEAM_STATE_PUBLISHER_HPP
#include <type_traits>
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /**
   * Publishes the most recent update to a value.
   * @param <T> The type of data to publish.
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
      template<typename VF, typename = std::enable_if_t<
        !std::is_base_of_v<StatePublisher, std::decay_t<VF>>>>
      explicit StatePublisher(VF&& value);

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

      using QueueWriter<T>::Break;
      using SnapshotPublisher<T, T>::With;
    private:
      mutable Threading::RecursiveMutex m_mutex;
      boost::optional<Type> m_value;
      QueueWriterPublisher<Type> m_publisher;
  };

  template<typename T>
  template<typename VF, typename>
  StatePublisher<T>::StatePublisher(VF&& value)
    : m_value(std::forward<VF>(value)) {}

  template<typename T>
  void StatePublisher<T>::With(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    if(m_value) {
      f(*m_value);
    } else {
      f(boost::none);
    }
  }

  template<typename T>
  void StatePublisher<T>::Monitor(ScopedQueueWriter<Type> queue,
      Out<boost::optional<Snapshot>> snapshot) const {
    auto lock = boost::lock_guard(m_mutex);
    *snapshot = m_value;
    m_publisher.Monitor(std::move(queue));
  }

  template<typename T>
  void StatePublisher<T>::With(const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename T>
  void StatePublisher<T>::Monitor(ScopedQueueWriter<Type> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    if(m_value) {
      queue.Push(*m_value);
    }
    m_publisher.Monitor(std::move(queue));
  }

  template<typename T>
  void StatePublisher<T>::Push(const Type& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_value = value;
    m_publisher.Push(*m_value);
  }

  template<typename T>
  void StatePublisher<T>::Push(Type&& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_value = std::move(value);
    m_publisher.Push(*m_value);
  }

  template<typename T>
  void StatePublisher<T>::Break(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_value = boost::none;
    m_publisher.Break(e);
  }
}

#endif
