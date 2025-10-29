#ifndef BEAM_QUEUE_READER_PUBLISHER_HPP
#define BEAM_QUEUE_READER_PUBLISHER_HPP
#include <atomic>
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Queues/ScopedQueueReader.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam {

  /**
   * Publishes values received from a QueueReader.
   * @tparam T The type of values to read.
   * @tparam Q The type of QueueReader to read from.
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

      void with(const std::function<void ()>& f) const override;
      void monitor(ScopedQueueWriter<Type> monitor) const override;
      using Publisher<T>::with;

    private:
      std::atomic_bool m_is_reading;
      ScopedQueueReader<T, Q> m_reader;
      QueueWriterPublisher<T> m_writer;
      RoutineHandler m_routine;

      void start();
      void loop();
  };

  template<typename Q>
  QueueReaderPublisher(Q&&) ->
    QueueReaderPublisher<typename dereference_t<Q>::Source>;

  template<typename T, typename Q>
  QueueReaderPublisher<T, Q>::QueueReaderPublisher(
    ScopedQueueReader<T, Q> reader)
    : m_is_reading(false),
      m_reader(std::move(reader)) {}

  template<typename T, typename Q>
  QueueReaderPublisher<T, Q>::~QueueReaderPublisher() {
    m_reader.close();
  }

  template<typename T, typename Q>
  void QueueReaderPublisher<T, Q>::with(const std::function<void ()>& f) const {
    m_writer.with(f);
  }

  template<typename T, typename Q>
  void QueueReaderPublisher<T, Q>::monitor(
      ScopedQueueWriter<Type> monitor) const {
    m_writer.monitor(std::move(monitor));
    const_cast<QueueReaderPublisher*>(this)->start();
  }

  template<typename T, typename Q>
  void QueueReaderPublisher<T, Q>::start() {
    if(!m_is_reading.exchange(true)) {
      m_routine = spawn(std::bind_front(&QueueReaderPublisher::loop, this));
    }
  }

  template<typename T, typename Q>
  void QueueReaderPublisher<T, Q>::loop() {
    try {
      while(true) {
        m_writer.push(m_reader.pop());
      }
    } catch(const std::exception&) {
      m_writer.close(std::current_exception());
      m_reader.close(std::current_exception());
    }
  }
}

#endif
