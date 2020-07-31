#ifndef BEAM_AGGREGATE_PUBLISHER_HPP
#define BEAM_AGGREGATE_PUBLISHER_HPP
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Utilities/SynchronizedList.hpp"

namespace Beam {

  /**
   * Aggregates Publishers together.
   * @param <P> The type of Publisher used to for unification.
   */
  template<typename P>
  class AggregatePublisher final : public Details::GetPublisherType<P>::type {
    public:
      using Type = typename P::Type;
      using Snapshot = typename Details::GetSnapshotType<P>::type;

      /**
       * Constructs an AggregatePublisher.
       * @param publisher Initializes the Publisher used to unify all aggregate
       *        Publishers.
       */
      template<typename PF>
      AggregatePublisher(PF&& publisher);

      ~AggregatePublisher() override;

      /**
       * Adds a Publisher to aggregate.
       * @param publisher The Publisher to add.
       */
      void Add(const Publisher<Type>& publisher);

      void Push(const Type& value) override;

      void Break() override;

      void Break(const std::exception_ptr& e) override;

      template<typename E>
      void Break(const E& e);

      void WithSnapshot(const std::function<
        void (boost::optional<const Snapshot&>)>& f) const override;

      void Monitor(std::shared_ptr<QueueWriter<Type>> monitor,
        Out<boost::optional<Snapshot>> snapshot) const override;

      void With(const std::function<void ()>& f) const override;

      void Monitor(std::shared_ptr<QueueWriter<Type>> monitor) const override;

    private:
      SynchronizedVector<std::shared_ptr<CallbackQueueWriter<Type>>>
        m_callbacks;
      GetOptionalLocalPtr<P> m_publisher;
  };

  template<typename P>
  template<typename PF>
  AggregatePublisher<P>::AggregatePublisher(PF&& publisher)
    : m_publisher(std::forward<PF>(publisher)) {}

  template<typename P>
  AggregatePublisher<P>::~AggregatePublisher() {
    Break();
  }

  template<typename P>
  void AggregatePublisher<P>::Add(const Publisher<Type>& publisher) {
    auto callback = std::make_shared<CallbackQueueWriter<Type>>(
      [=] (const Type& value) {
        m_publisher->Push(value);
      });
    publisher.Monitor(callback);
    m_callbacks.PushBack(callback);
  }

  template<typename P>
  void AggregatePublisher<P>::Push(const Type& value) {
    m_publisher->Push(value);
  }

  template<typename P>
  void AggregatePublisher<P>::Break() {
    Break(PipeBrokenException());
  }

  template<typename P>
  void AggregatePublisher<P>::Break(const std::exception_ptr& e) {
    m_publisher->Break(e);
    m_callbacks.Clear();
  }

  template<typename P>
  template<typename E>
  void AggregatePublisher<P>::Break(const E& e) {
    Break(std::make_exception_ptr(e));
  }

  template<typename P>
  void AggregatePublisher<P>::WithSnapshot(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    m_publisher->WithSnapshot(f);
  }

  template<typename P>
  void AggregatePublisher<P>::Monitor(
      std::shared_ptr<QueueWriter<Type>> monitor,
      Out<boost::optional<Snapshot>> snapshot) const {
    m_publisher->Monitor(monitor, Store(snapshot));
  }

  template<typename P>
  void AggregatePublisher<P>::With(const std::function<void ()>& f) const {
    m_publisher->With(f);
  }

  template<typename P>
  void AggregatePublisher<P>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue) const {
    m_publisher->Monitor(queue);
  }
}

#endif
