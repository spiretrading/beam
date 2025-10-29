#ifndef BEAM_SEQUENCE_PUBLISHER_HPP
#define BEAM_SEQUENCE_PUBLISHER_HPP
#include <vector>
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Queues/WeakQueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Publishes a sequence of data.
   * @tparam T The type of data to publish.
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
      template<Initializes<S> SF,
        typename = disable_copy_constructor_t<SequencePublisher, SF>>
      explicit SequencePublisher(SF&& sequence);

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
      using SnapshotPublisher<T, S>::with;

    private:
      mutable RecursiveMutex m_mutex;
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
  auto make_sequence_publisher_adaptor(PF&& publisher) {
    using Publisher = std::remove_cvref_t<PF>;
    using Type = typename dereference_t<Publisher>::Type;
    auto holder = std::make_shared<
      std::pair<SequencePublisher<Type>, boost::optional<Publisher>>>();
    holder->second.emplace(std::forward<PF>(publisher));
    auto sequence_publisher =
      std::shared_ptr<SequencePublisher<Type>>(holder, &holder->first);
    (*holder->second)->monitor(make_weak_queue_writer(sequence_publisher));
    struct Deleter {
      std::shared_ptr<SequencePublisher<Type>> m_publisher;

      void operator ()(SequencePublisher<Type>* value) {
        m_publisher->close();
        m_publisher = nullptr;
      }
    };
    return std::unique_ptr<SequencePublisher<Type>, Deleter>(
      sequence_publisher.get(), Deleter(sequence_publisher));
  }

  template<typename T, typename S>
  template<Initializes<S> SF, typename>
  SequencePublisher<T, S>::SequencePublisher(SF&& sequence)
    : m_sequence(std::forward<SF>(sequence)) {}

  template<typename T, typename S>
  void SequencePublisher<T, S>::with(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f(m_sequence);
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::monitor(ScopedQueueWriter<Type> monitor,
      Out<boost::optional<Snapshot>> snapshot) const {
    auto lock = boost::lock_guard(m_mutex);
    *snapshot = m_sequence;
    m_publisher.monitor(std::move(monitor));
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::with(const std::function<void ()>& f) const {
    auto lock = boost::lock_guard(m_mutex);
    f();
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::monitor(ScopedQueueWriter<Type> queue) const {
    auto lock = boost::lock_guard(m_mutex);
    for(auto& i : m_sequence) {
      queue.push(i);
    }
    m_publisher.monitor(std::move(queue));
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::push(const Type& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_sequence.push_back(value);
    m_publisher.push(value);
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::push(Type&& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_sequence.push_back(value);
    m_publisher.push(std::move(value));
  }

  template<typename T, typename S>
  void SequencePublisher<T, S>::close(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    m_publisher.close(e);
  }
}

#endif
