#ifndef BEAM_QUEUE_READER_PUBLISHER_HPP
#define BEAM_QUEUE_READER_PUBLISHER_HPP
#include <atomic>
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Queues/ScopedQueueReader.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam {

  /**
   * Publishes values received from a QueueReader.
   * @param <T> The type of values to read.
   * @param <Q> The type of QueueReader to read from.
   */
  template<typename T, typename Q = std::shared_ptr<QueueReader<T>>>
  class QueueReaderPublisher final : public Publisher<T> {
    public:
      using Type = T;

      /**
       * Constructs a publisher.
       * @param reader The QueueReader to read from.
       */
      explicit QueueReaderPublisher(ScopedQueueReader<T, Q> reader);

      ~QueueReaderPublisher() override;

      void With(const std::function<void ()>& f) const override;

      void Monitor(ScopedQueueWriter<Type> monitor) const override;

      using Publisher<T>::With;
    private:
      std::atomic_bool m_isReading;
      ScopedQueueReader<T, Q> m_reader;
      QueueWriterPublisher<T> m_writer;
      Routines::RoutineHandler m_routine;

      void Start();
      void ReadLoop();
  };

  template<typename Q>
  QueueReaderPublisher(Q&&) ->
    QueueReaderPublisher<typename GetTryDereferenceType<Q>::Source>;

  template<typename T, typename Q>
  QueueReaderPublisher<T, Q>::QueueReaderPublisher(
    ScopedQueueReader<T, Q> reader)
    : m_isReading(false),
      m_reader(std::move(reader)) {}

  template<typename T, typename Q>
  QueueReaderPublisher<T, Q>::~QueueReaderPublisher() {
    m_reader.Break();
  }

  template<typename T, typename Q>
  void QueueReaderPublisher<T, Q>::With(const std::function<void ()>& f) const {
    m_writer.With(f);
  }

  template<typename T, typename Q>
  void QueueReaderPublisher<T, Q>::Monitor(
      ScopedQueueWriter<Type> monitor) const {
    m_writer.Monitor(std::move(monitor));
    const_cast<QueueReaderPublisher*>(this)->Start();
  }

  template<typename T, typename Q>
  void QueueReaderPublisher<T, Q>::Start() {
    if(!m_isReading.exchange(true)) {
      m_routine = Routines::Spawn([=] { ReadLoop(); });
    }
  }

  template<typename T, typename Q>
  void QueueReaderPublisher<T, Q>::ReadLoop() {
    try {
      while(true) {
        m_writer.Push(m_reader.Pop());
      }
    } catch(const std::exception&) {
      m_writer.Break(std::current_exception());
      m_reader.Break(std::current_exception());
    }
  }
}

#endif
