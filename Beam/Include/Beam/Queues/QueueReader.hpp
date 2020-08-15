#ifndef BEAM_QUEUE_READER_HPP
#define BEAM_QUEUE_READER_HPP
#include <type_traits>
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Queues/BaseQueue.hpp"
#include "Beam/Queues/Queues.hpp"

namespace Beam {

  /**
   * Interface for the read-only side of a Queue.
   * @param <T> The data to read from the Queue.
   */
  template<typename T>
  class QueueReader : public virtual BaseQueue {
    public:

      /** The type being read. */
      using Source = T;

      /**
       * Returns the first value in the queue and pops it, blocking until a
       * value is available.
       */
      virtual Source Pop() = 0;

      /**
       * Returns the first value in the queue if one is available and pops it
       * without blocking, otherwise returns <i>boost::none</i>.
       */
      virtual boost::optional<Source> TryPop() = 0;
  };

  /**
   * Flushes the contents of a QueueReader into an iterator.
   * @param queue The QueueReader to flush.
   * @param destination An iterator to the first position to flush the
   *        <i>queue<i> to.
   */
  template<typename Queue, typename Iterator>
  std::enable_if_t<std::is_base_of_v<QueueReader<typename GetTryDereferenceType<
      Queue>::Source>, GetTryDereferenceType<Queue>>> Flush(const Queue& queue,
      Iterator destination) {
    try {
      while(true) {
        *destination = queue->Pop();
        ++destination;
      }
    } catch(const std::exception&) {}
  }

  template<typename Queue, typename F>
  std::enable_if_t<std::is_base_of_v<QueueReader<typename GetTryDereferenceType<
      Queue>::Source>, GetTryDereferenceType<Queue>>> ForEach(
      const Queue& queue, F&& f) {
    try {
      while(true) {
        f(queue->Pop());
      }
    } catch(const std::exception&) {}
  }

  template<typename Queue, typename F1, typename F2>
  void Monitor(const Queue& queue, F1&& valueCallback, F2&& breakCallback) {
    try {
      while(true) {
        valueCallback(queue->Pop());
      }
    } catch(const std::exception&) {
      breakCallback(std::current_exception());
    }
  }
}

#endif
