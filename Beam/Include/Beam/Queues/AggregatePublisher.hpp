#ifndef BEAM_AGGREGATEPUBLISHER_HPP
#define BEAM_AGGREGATEPUBLISHER_HPP
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Utilities/SynchronizedList.hpp"

namespace Beam {

  /*! \class AggregatePublisher
      \brief Aggregates Publishers together.
      \tparam PublisherType The type of Publisher used to for unification.
   */
  template<typename PublisherType>
  class AggregatePublisher :
      public Details::GetPublisherType<PublisherType>::type {
    public:
      using Type = typename PublisherType::Type;
      using Snapshot = typename Details::GetSnapshotType<PublisherType>::type;

      //! Constructs an AggregatePublisher.
      /*!
        \param publisher Initializes the Publisher used to unify all aggregate
               Publishers.
      */
      template<typename PublisherForward>
      AggregatePublisher(PublisherForward&& publisher);

      virtual ~AggregatePublisher() override final;

      //! Adds a Publisher to aggregate.
      /*!
        \param publisher The Publisher to add.
      */
      void Add(const Publisher<Type>& publisher);

      virtual void Push(const Type& value) final;

      virtual void Break() final;

      virtual void Break(const std::exception_ptr& e) final;

      template<typename E>
      void Break(const E& e);

      virtual void WithSnapshot(const std::function<
        void (boost::optional<const Snapshot&>)>& f) const override final;

      virtual void Monitor(std::shared_ptr<QueueWriter<Type>> monitor,
        Out<boost::optional<Snapshot>> snapshot) const override final;

      virtual void With(const std::function<void ()>& f) const override final;

      virtual void Monitor(
        std::shared_ptr<QueueWriter<Type>> monitor) const override final;

    private:
      typename OptionalLocalPtr<PublisherType>::type m_publisher;
      SynchronizedVector<std::shared_ptr<CallbackWriterQueue<Type>>>
        m_callbacks;
  };

  template<typename PublisherType>
  template<typename PublisherForward>
  AggregatePublisher<PublisherType>::AggregatePublisher(
      PublisherForward&& publisher)
      : m_publisher(std::forward<PublisherForward>(publisher)) {}

  template<typename PublisherType>
  AggregatePublisher<PublisherType>::~AggregatePublisher() {
    Break();
  }

  template<typename PublisherType>
  void AggregatePublisher<PublisherType>::Add(
      const Publisher<Type>& publisher) {
    std::shared_ptr<CallbackWriterQueue<Type>> callback =
      std::make_shared<CallbackWriterQueue<Type>>(
        [=] (const Type& value) {
          m_publisher->Push(value);
        });
    publisher.Monitor(callback);
    m_callbacks.PushBack(callback);
  }

  template<typename PublisherType>
  void AggregatePublisher<PublisherType>::Push(const Type& value) {
    m_publisher->Push(value);
  }

  template<typename PublisherType>
  void AggregatePublisher<PublisherType>::Break() {
    Break(PipeBrokenException());
  }

  template<typename PublisherType>
  void AggregatePublisher<PublisherType>::Break(const std::exception_ptr& e) {
    m_publisher->Break(e);
    m_callbacks.Clear();
  }

  template<typename PublisherType>
  template<typename E>
  void AggregatePublisher<PublisherType>::Break(const E& e) {
    Break(std::make_exception_ptr(e));
  }

  template<typename PublisherType>
  void AggregatePublisher<PublisherType>::WithSnapshot(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    m_publisher->WithSnapshot(f);
  }

  template<typename PublisherType>
  void AggregatePublisher<PublisherType>::Monitor(
      std::shared_ptr<QueueWriter<Type>> monitor,
      Out<boost::optional<Snapshot>> snapshot) const {
    m_publisher->Monitor(monitor, Store(snapshot));
  }

  template<typename PublisherType>
  void AggregatePublisher<PublisherType>::With(
      const std::function<void ()>& f) const {
    m_publisher->With(f);
  }

  template<typename PublisherType>
  void AggregatePublisher<PublisherType>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue) const {
    m_publisher->Monitor(queue);
  }
}

#endif
