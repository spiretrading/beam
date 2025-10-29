#ifndef BEAM_QUEUE_PIPE_HPP
#define BEAM_QUEUE_PIPE_HPP
#include "Beam/Queues/ScopedQueueReader.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam {

  /**
   * Pushes the values popped off a QueueReader onto a QueueWriter.
   * @tparam T The type of data to pop off the QueueReader.
   * @tparam U The type of data to push onto the QueueWriter.
   */
  template<typename T, typename U = T>
  class QueuePipe : public BaseQueue {
    public:

      /** The type of data to pop off the QueueReader. */
      using Source = T;

      /** The type of data to push onto the QueueWriter. */
      using Target = U;

      /**
       * Constructs a QueuePipe.
       * @param reader The QueueReader to pop from.
       * @param writer The QueueWriter to push to.
       */
      QueuePipe(
        ScopedQueueReader<Source> reader, ScopedQueueWriter<Target> writer);

      ~QueuePipe() override;

      void close(const std::exception_ptr& e) override;
      using BaseQueue::close;

    private:
      ScopedQueueReader<Source> m_reader;
      ScopedQueueWriter<Target> m_writer;
      RoutineHandler m_routine;

      void loop();
  };

  template<typename T, typename U>
  QueuePipe(T&&, U&&) -> QueuePipe<
    typename dereference_t<T>::Source, typename dereference_t<U>::Target>;

  template<typename T, typename U>
  QueuePipe<T, U>::QueuePipe(
    ScopedQueueReader<Source> reader, ScopedQueueWriter<Target> writer)
    : m_reader(std::move(reader)),
      m_writer(std::move(writer)),
      m_routine(spawn(std::bind_front(&QueuePipe::loop, this))) {}

  template<typename T, typename U>
  QueuePipe<T, U>::~QueuePipe() {
    close();
  }

  template<typename T, typename U>
  void QueuePipe<T, U>::close(const std::exception_ptr& e) {
    m_writer.close(e);
    m_reader.close(e);
  }

  template<typename T, typename U>
  void QueuePipe<T, U>::loop() {
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
