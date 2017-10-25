#ifndef BEAM_SEQUENCEPUBLISHER_HPP
#define BEAM_SEQUENCEPUBLISHER_HPP
#include <vector>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /*! \class SequencePublisher
      \brief Publishes a sequence of data.
      \tparam T The type of data to publish.
   */
  template<typename T, typename SequenceType = std::vector<T>>
  class SequencePublisher : public SnapshotPublisher<T, SequenceType>,
      public QueueWriter<T> {
    public:
      using Type = typename SnapshotPublisher<T, SequenceType>::Type;
      using Snapshot = typename SnapshotPublisher<T, SequenceType>::Snapshot;

      //! Constructs a SequencePublisher.
      SequencePublisher() = default;

      //! Constructs a SequencePublisher.
      /*!
        \param sequence Initializes the sequence.
      */
      template<typename SequenceForward>
      SequencePublisher(SequenceForward&& sequence);

      virtual ~SequencePublisher() override final;

      virtual void WithSnapshot(const std::function<
        void (boost::optional<const Snapshot&>)>& f) const override final;

      virtual void Monitor(std::shared_ptr<QueueWriter<T>> monitor,
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
      LocalPtr<SequenceType> m_sequence;
      MultiQueueWriter<T> m_queue;
  };

  template<typename T, typename SequenceType>
  template<typename SequenceForward>
  SequencePublisher<T, SequenceType>::SequencePublisher(
      SequenceForward&& sequence)
      : m_sequence(std::forward<SequenceForward>(sequence)) {}

  template<typename T, typename SequenceType>
  SequencePublisher<T, SequenceType>::~SequencePublisher() {
    Break();
  }

  template<typename T, typename SequenceType>
  void SequencePublisher<T, SequenceType>::WithSnapshot(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    f(*m_sequence);
  }

  template<typename T, typename SequenceType>
  void SequencePublisher<T, SequenceType>::Monitor(
      std::shared_ptr<QueueWriter<T>> monitor,
      Out<boost::optional<Snapshot>> snapshot) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    *snapshot = *m_sequence;
    m_queue.Monitor(monitor);
  }

  template<typename T, typename SequenceType>
  void SequencePublisher<T, SequenceType>::With(
      const std::function<void ()>& f) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    f();
  }

  template<typename T, typename SequenceType>
  void SequencePublisher<T, SequenceType>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    for(auto& i : *m_sequence) {
      queue->Push(i);
    }
    m_queue.Monitor(queue);
  }

  template<typename T, typename SequenceType>
  void SequencePublisher<T, SequenceType>::Push(const T& value) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    m_sequence->push_back(value);
    m_queue.Push(value);
  }

  template<typename T, typename SequenceType>
  void SequencePublisher<T, SequenceType>::Push(T&& value) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    m_sequence->push_back(value);
    m_queue.Push(std::move(value));
  }

  template<typename T, typename SequenceType>
  void SequencePublisher<T, SequenceType>::Break(const std::exception_ptr& e) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    m_queue.Break(e);
  }
}

#endif
