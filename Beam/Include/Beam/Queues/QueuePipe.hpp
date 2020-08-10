#ifndef BEAM_QUEUE_PIPE_HPP
#define BEAM_QUEUE_PIPE_HPP
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/ScopedQueueReader.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam {

  /**
   * Pushes the values popped off a QueueReader onto a QueueWriter.
   * @param <T> The type of data to pop off the QueueReader.
   * @param <U> The type of data to push onto the QueueWriter.
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
      QueuePipe(ScopedQueueReader<Source> reader,
        ScopedQueueWriter<Target> writer);

      ~QueuePipe() override;

      void Break(const std::exception_ptr& e) override;

      using BaseQueue::Break;

    private:
      ScopedQueueReader<Source> m_reader;
      ScopedQueueWriter<Target> m_writer;
      Routines::RoutineHandler m_routine;

      void ReadLoop();
  };

  template<typename T, typename U>
  QueuePipe(T&&, U&&) -> QueuePipe<typename GetTryDereferenceType<T>::Source,
    typename GetTryDereferenceType<U>::Target>;

  template<typename T, typename U>
  QueuePipe<T, U>::QueuePipe(ScopedQueueReader<Source> reader,
    ScopedQueueWriter<Target> writer)
    : m_reader(std::move(reader)),
      m_writer(std::move(writer)),
      m_routine(Routines::Spawn([=] { ReadLoop(); })) {}

  template<typename T, typename U>
  QueuePipe<T, U>::~QueuePipe() {
    Break();
  }

  template<typename T, typename U>
  void QueuePipe<T, U>::Break(const std::exception_ptr& e) {
    m_writer.Break(e);
    m_reader.Break(e);
  }

  template<typename T, typename U>
  void QueuePipe<T, U>::ReadLoop() {
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
