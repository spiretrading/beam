#ifndef BEAM_SEQUENCE_PUBLISHER_HPP
#define BEAM_SEQUENCE_PUBLISHER_HPP
#include <vector>
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /**
   * Publishes a sequence of data.
   * @param <T> The type of data to publish.
   */
  template<typename T, typename S = std::vector<T>>
  class SequencePublisher final : public SnapshotPublisher<T, S>,
      public QueueWriter<T> {
    public:
      using Type = typename SnapshotPublisher<T, S>::Type;
      using Snapshot = typename SnapshotPublisher<T, S>::Snapshot;

      /** Constructs a SequencePublisher. */
      SequencePublisher() = default;

      /**
       * Constructs a SequencePublisher.
       * @param sequence Initializes the sequence.
       */
      template<typename SF>
      SequencePublisher(SF&& sequence);

      void WithSnapshot(const std::function<
        void (boost::optional<const Snapshot&>)>& f) const override;

      void Monitor(std::shared_ptr<QueueWriter<Type>> monitor,
        Out<boost::optional<Snapshot>> snapshot) const override;

      void With(const std::function<void ()>& f) const override;

      void Monitor(std::shared_ptr<QueueWriter<Type>> monitor) const override;

      void Push(const Type& value) override;

      void Push(Type&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<Type>::Break;

    private:
      mutable Threading::RecursiveMutex m_mutex;
      Snapshot m_sequence;
      MultiQueueWriter<Type> m_queue;
  };

  template<typename T, typename S>
  template<typename SF>
  SequencePublisher<T, S>::SequencePublisher(SF&& sequence)
    : m_sequence(std::forward<SF>(sequence)) {}

  template<typename T, typename S>
  void SequencePublisher<T, S>::WithSnapshot(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f(m_sequence);
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::Monitor(
      std::shared_ptr<QueueWriter<Type>> monitor,
      Out<boost::optional<Snapshot>> snapshot) const {
    auto lock = boost::lock_guard(m_mutex);
    *snapshot = m_sequence;
    m_queue.Monitor(std::move(monitor));
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::With(const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    for(auto& i : m_sequence) {
      queue->Push(i);
    }
    m_queue.Monitor(std::move(queue));
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::Push(const Type& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_sequence.push_back(value);
    m_queue.Push(value);
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::Push(Type&& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_sequence.push_back(value);
    m_queue.Push(std::move(value));
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::Break(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_queue.Break(e);
  }
}

#endif
