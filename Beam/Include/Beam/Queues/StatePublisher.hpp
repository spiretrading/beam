#ifndef BEAM_STATEPUBLISHER_HPP
#define BEAM_STATEPUBLISHER_HPP
#include "Beam/Pointers/DelayPtr.hpp"
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /*! \class StatePublisher
      \brief Publishes the most recent update to a value.
      \tparam T The type of data to publish.
   */
  template<typename T>
  class StatePublisher : public SnapshotPublisher<T, T>, public QueueWriter<T> {
    public:
      using Type = typename SnapshotPublisher<T, T>::Type;
      using Snapshot = typename SnapshotPublisher<T, T>::Snapshot;

      //! Constructs a StatePublisher.
      StatePublisher() = default;

      //! Constructs a StatePublisher.
      /*!
        \param value The initial state of the value to publish.
      */
      template<typename ValueForward>
      StatePublisher(ValueForward&& value);

      ~StatePublisher() override final;

      virtual void WithSnapshot(const std::function<
        void (boost::optional<const Snapshot&>)>& f) const override final;

      virtual void Monitor(std::shared_ptr<QueueWriter<Type>> monitor,
        Out<boost::optional<Snapshot>> snapshot) const override final;

      virtual void With(const std::function<void ()>& f) const override final;

      virtual void Monitor(
        std::shared_ptr<QueueWriter<Type>> monitor) const override final;

      virtual void Push(const T& value) override final;

      virtual void Push(T&& value) override final;

      virtual void Break(const std::exception_ptr& e) override final;

      using QueueWriter<T>::Break;
    private:
      mutable Threading::RecursiveMutex m_mutex;
      Beam::DelayPtr<T> m_value;
      MultiQueueWriter<T> m_queue;
  };

  template<typename T>
  template<typename ValueForward>
  StatePublisher<T>::StatePublisher(ValueForward&& value)
      : m_value(std::forward<ValueForward>(value)) {}

  template<typename T>
  StatePublisher<T>::~StatePublisher() {
    Break();
  }

  template<typename T>
  void StatePublisher<T>::WithSnapshot(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    if(m_value.IsInitialized()) {
      f(*m_value);
    } else {
      f(boost::optional<const Snapshot&>());
    }
  }

  template<typename T>
  void StatePublisher<T>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue,
      Out<boost::optional<Snapshot>> snapshot) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    if(m_value.IsInitialized()) {
      *snapshot = *m_value;
    } else {
      *snapshot = boost::optional<Snapshot>();
    }
    m_queue.Monitor(queue);
  }

  template<typename T>
  void StatePublisher<T>::With(const std::function<void ()>& f) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    f();
  }

  template<typename T>
  void StatePublisher<T>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    if(m_value.IsInitialized()) {
      queue->Push(*m_value);
    }
    m_queue.Monitor(queue);
  }

  template<typename T>
  void StatePublisher<T>::Push(const T& value) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    if(m_value.IsInitialized()) {
      *m_value = value;
    } else {
      m_value.Initialize(value);
    }
    m_queue.Push(*m_value);
  }

  template<typename T>
  void StatePublisher<T>::Push(T&& value) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    if(m_value.IsInitialized()) {
      *m_value = std::move(value);
    } else {
      m_value.Initialize(std::move(value));
    }
    m_queue.Push(*m_value);
  }

  template<typename T>
  void StatePublisher<T>::Break(const std::exception_ptr& e) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    m_value.Reset();
    m_queue.Break(e);
  }
}

#endif
