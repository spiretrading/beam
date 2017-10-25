#ifndef BEAM_QUEUEPUBLISHER_HPP
#define BEAM_QUEUEPUBLISHER_HPP
#include <boost/atomic/atomic.hpp>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam {
namespace Details {
  template<bool Enabled = true>
  struct QueuePublisherWithSnapshot {
    template<typename Publisher, typename F>
    void operator ()(const Publisher& publisher, const F& f) {
      publisher.WithSnapshot(f);
    }
  };

  template<>
  struct QueuePublisherWithSnapshot<false> {
    template<typename Publisher, typename F>
    void operator ()(const Publisher& publisher, const F& f) {}
  };

  template<bool Enabled = true>
  struct QueuePublisherMonitor {
    template<typename Publisher, typename Monitor, typename Snapshot>
    void operator ()(const Publisher& publisher,
        std::shared_ptr<Monitor> monitor, Out<Snapshot> snapshot) {
      publisher.Monitor(monitor, Store(snapshot));
    }
  };

  template<>
  struct QueuePublisherMonitor<false> {
    template<typename Publisher, typename Monitor, typename Snapshot>
    void operator ()(const Publisher& publisher,
        std::shared_ptr<Monitor> monitor, Out<Snapshot> snapshot) {}
  };
}

  /*! \class QueuePublisher
      \brief Publishes data received from a Queue.
      \tparam PublisherType The type of Publisher to adapt.
   */
  template<typename PublisherType>
  class QueuePublisher : public Details::GetPublisherType<PublisherType>::type {
    public:
      using Type = typename PublisherType::Type;
      using Snapshot = typename Details::GetSnapshotType<PublisherType>::type;

      //! Constructs a QueuePublisher.
      /*!
        \param queue The Queue to publish.
      */
      QueuePublisher(const std::shared_ptr<QueueReader<Type>>& queue);

      virtual ~QueuePublisher() override final;

      //! Breaks the Queue.
      void Break();

      //! Breaks the Queue with a specified exception.
      /*!
        \param exception The cause for the break.
      */
      void Break(const std::exception_ptr& exception);

      //! Breaks the Queue with a specified exception.
      /*!
        \param exception The cause for the break.
      */
      template<typename E>
      void Break(const E& exception);

      virtual void WithSnapshot(const std::function<
        void (boost::optional<const Snapshot&>)>& f) const final;

      virtual void Monitor(std::shared_ptr<QueueWriter<Type>> monitor,
        Out<boost::optional<Snapshot>> snapshot) const final;

      virtual void With(const std::function<void ()>& f) const override final;

      virtual void Monitor(
        std::shared_ptr<QueueWriter<Type>> monitor) const override final;

    private:
      mutable boost::atomic_bool m_isReading;
      mutable PublisherType m_publisher;
      mutable std::shared_ptr<QueueReader<Type>> m_queue;
      mutable Routines::RoutineHandler m_readLoop;

      void StartReadLoop() const;
      void ReadLoop() const;
  };

  template<typename PublisherType>
  QueuePublisher<PublisherType>::QueuePublisher(
      const std::shared_ptr<QueueReader<Type>>& queue)
      : m_isReading(false),
        m_queue(queue) {}

  template<typename PublisherType>
  QueuePublisher<PublisherType>::~QueuePublisher() {
    Break();
  }

  template<typename PublisherType>
  void QueuePublisher<PublisherType>::Break() {
    m_queue->Break();
    m_publisher.Break();
  }

  template<typename PublisherType>
  void QueuePublisher<PublisherType>::Break(
      const std::exception_ptr& exception) {
    m_queue->Break(exception);
    m_publisher.Break(exception);
  }

  template<typename PublisherType>
  template<typename E>
  void QueuePublisher<PublisherType>::Break(const E& exception) {
    m_queue->Break(exception);
    m_publisher.Break(exception);
  }

  template<typename PublisherType>
  void QueuePublisher<PublisherType>::WithSnapshot(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    Details::QueuePublisherWithSnapshot<
      std::is_base_of<BaseSnapshotPublisher, PublisherType>::value>()(
      m_publisher, f);
  }

  template<typename PublisherType>
  void QueuePublisher<PublisherType>::Monitor(
      std::shared_ptr<QueueWriter<Type>> monitor,
      Out<boost::optional<Snapshot>> snapshot) const {
    Details::QueuePublisherMonitor<
      std::is_base_of<BaseSnapshotPublisher, PublisherType>::value>()(
      m_publisher, monitor, Store(snapshot));
    StartReadLoop();
  }

  template<typename PublisherType>
  void QueuePublisher<PublisherType>::With(
      const std::function<void ()>& f) const {
    m_publisher.With(f);
  }

  template<typename PublisherType>
  void QueuePublisher<PublisherType>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue) const {
    m_publisher.Monitor(queue);
    StartReadLoop();
  }

  template<typename PublisherType>
  void QueuePublisher<PublisherType>::StartReadLoop() const {
    bool isReading = m_isReading.exchange(true);
    if(!isReading) {
      m_readLoop = Routines::Spawn(std::bind(&QueuePublisher::ReadLoop, this));
    }
  }

  template<typename PublisherType>
  void QueuePublisher<PublisherType>::ReadLoop() const {
    try {
      while(true) {
        auto value = m_queue->Top();
        m_queue->Pop();
        m_publisher.Push(std::move(value));
      }
    } catch(const std::exception&) {
      m_publisher.Break(std::current_exception());
    }
  }
}

#endif
