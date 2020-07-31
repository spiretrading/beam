#ifndef BEAM_QUEUE_PUBLISHER_HPP
#define BEAM_QUEUE_PUBLISHER_HPP
#include <atomic>
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
      publisher.Monitor(std::move(monitor), Store(snapshot));
    }
  };

  template<>
  struct QueuePublisherMonitor<false> {
    template<typename Publisher, typename Monitor, typename Snapshot>
    void operator ()(const Publisher& publisher,
      std::shared_ptr<Monitor> monitor, Out<Snapshot> snapshot) {}
  };
}

  /**
   * Publishes data received from a Queue.
   * @param <P> The type of Publisher to adapt.
   */
  template<typename P>
  class QueuePublisher final : public Details::GetPublisherType<P>::type {
    public:
      using Type = typename P::Type;
      using Snapshot = typename Details::GetSnapshotType<P>::type;

      /**
       * Constructs a QueuePublisher.
       * @param queue The Queue to publish.
       */
      QueuePublisher(std::shared_ptr<QueueReader<Type>> queue);

      ~QueuePublisher() override;

      /** Breaks the Queue. */
      void Break();

      /**
       * Breaks the Queue with a specified exception.
       * @param exception The cause for the break.
       */
      void Break(const std::exception_ptr& exception);

      /**
       * Breaks the Queue with a specified exception.
       * @param exception The cause for the break.
       */
      template<typename E>
      void Break(const E& exception);

      void WithSnapshot(const std::function<
        void (boost::optional<const Snapshot&>)>& f) const override;

      void Monitor(std::shared_ptr<QueueWriter<Type>> monitor,
        Out<boost::optional<Snapshot>> snapshot) const override;

      void With(const std::function<void ()>& f) const override;

      void Monitor(std::shared_ptr<QueueWriter<Type>> monitor) const override;

    private:
      mutable std::atomic_bool m_isReading;
      mutable P m_publisher;
      mutable std::shared_ptr<QueueReader<Type>> m_queue;
      mutable Routines::RoutineHandler m_readLoop;

      void StartReadLoop() const;
      void ReadLoop() const;
  };

  template<typename P>
  QueuePublisher<P>::QueuePublisher(std::shared_ptr<QueueReader<Type>> queue)
    : m_isReading(false),
      m_queue(std::move(queue)) {}

  template<typename P>
  QueuePublisher<P>::~QueuePublisher() {
    Break();
  }

  template<typename P>
  void QueuePublisher<P>::Break() {
    m_queue->Break();
    m_publisher.Break();
  }

  template<typename P>
  void QueuePublisher<P>::Break(const std::exception_ptr& exception) {
    m_queue->Break(exception);
    m_publisher.Break(exception);
  }

  template<typename P>
  template<typename E>
  void QueuePublisher<P>::Break(const E& exception) {
    m_queue->Break(exception);
    m_publisher.Break(exception);
  }

  template<typename P>
  void QueuePublisher<P>::WithSnapshot(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    Details::QueuePublisherWithSnapshot<
      std::is_base_of_v<BaseSnapshotPublisher, P>>()(m_publisher, f);
  }

  template<typename P>
  void QueuePublisher<P>::Monitor(std::shared_ptr<QueueWriter<Type>> monitor,
      Out<boost::optional<Snapshot>> snapshot) const {
    Details::QueuePublisherMonitor<
      std::is_base_of_v<BaseSnapshotPublisher, P>>()(m_publisher,
      std::move(monitor), Store(snapshot));
    StartReadLoop();
  }

  template<typename P>
  void QueuePublisher<P>::With(const std::function<void ()>& f) const {
    m_publisher.With(f);
  }

  template<typename P>
  void QueuePublisher<P>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue) const {
    m_publisher.Monitor(std::move(queue));
    StartReadLoop();
  }

  template<typename P>
  void QueuePublisher<P>::StartReadLoop() const {
    auto isReading = m_isReading.exchange(true);
    if(!isReading) {
      m_readLoop = Routines::Spawn(std::bind(&QueuePublisher::ReadLoop, this));
    }
  }

  template<typename P>
  void QueuePublisher<P>::ReadLoop() const {
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
