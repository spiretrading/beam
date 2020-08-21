#ifndef BEAM_SEQUENCE_PUBLISHER_HPP
#define BEAM_SEQUENCE_PUBLISHER_HPP
#include <vector>
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Queues/WeakQueueWriter.hpp"
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
      template<typename SF, typename = std::enable_if_t<
        !std::is_base_of_v<StatePublisher, std::decay_t<SF>>>>
      explicit SequencePublisher(SF&& sequence);

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
      using SnapshotPublisher<T, S>::With;
    private:
      mutable Threading::RecursiveMutex m_mutex;
      Snapshot m_sequence;
      QueueWriterPublisher<Type> m_publisher;
  };

  /**
   * Makes a SequencePublisher that publishes monitored values.
   * @param publisher The publisher to monitor.
   * @return A SequencePublisher that publishes all monitored values from the
   *         the specified <i>publisher</i>.
   */
  template<typename PF>
  auto MakeSequencePublisherAdaptor(PF&& publisher) {
    using Publisher = std::remove_reference_t<PF>;
    using Type = typename GetTryDereferenceType<Publisher>::Type;
    auto holder = std::make_shared<std::tuple<SequencePublisher<Type>,
      boost::optional<Publisher>>>();
    std::get<1>(*holder).emplace(std::forward<PF>(publisher));
    auto sequencePublisher = std::shared_ptr<SequencePublisher<Type>>(holder,
      &std::get<0>(*holder));
    (*std::get<1>(*holder))->Monitor(MakeWeakQueueWriter(
      std::static_pointer_cast<QueueWriter<Type>>(sequencePublisher)));
    return sequencePublisher;
  }

  template<typename T, typename S>
  template<typename SF, typename>
  SequencePublisher<T, S>::SequencePublisher(SF&& sequence)
    : m_sequence(std::forward<SF>(sequence)) {}

  template<typename T, typename S>
  void SequencePublisher<T, S>::With(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f(m_sequence);
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::Monitor(ScopedQueueWriter<Type> monitor,
      Out<boost::optional<Snapshot>> snapshot) const {
    auto lock = boost::lock_guard(m_mutex);
    *snapshot = m_sequence;
    m_publisher.Monitor(std::move(monitor));
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::With(const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::Monitor(ScopedQueueWriter<Type> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    for(auto& i : m_sequence) {
      queue.Push(i);
    }
    m_publisher.Monitor(std::move(queue));
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::Push(const Type& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_sequence.push_back(value);
    m_publisher.Push(value);
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::Push(Type&& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_sequence.push_back(value);
    m_publisher.Push(std::move(value));
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::Break(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_publisher.Break(e);
  }
}

#endif
