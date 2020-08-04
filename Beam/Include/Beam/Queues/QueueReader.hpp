#ifndef BEAM_QUEUE_READER_HPP
#define BEAM_QUEUE_READER_HPP
#include <boost/optional/optional.hpp>
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
      using Target = T;

      /**
       * Returns the first value in the queue, blocking until a value is
       * available.
       */
      virtual Target Pop() = 0;

      /**
       * Returns the first value in the queue if one is available without
       * blocking, otherwise returns <i>boost::none</i>.
       */
      virtual boost::optional<Target> TryPop() = 0;
  };

  /**
   * Flushes the contents of a QueueReader into an iterator.
   * @param queue The QueueReader to flush.
   * @param destination An iterator to the first position to flush the
   *        <i>queue<i> to.
   */
  template<typename Queue, typename Iterator>
  void FlushQueue(const Queue& queue, Iterator destination) {
    try {
      while(true) {
        *destination = queue->Top();
        queue->Pop();
        ++destination;
      }
    } catch(const std::exception&) {}
  }

  template<typename Queue, typename F1, typename F2>
  void Monitor(const Queue& queue, F1&& valueCallback, F2&& breakCallback) {
    try {
      while(true) {
        valueCallback(queue->Top());
        queue->Pop();
      }
    } catch(const std::exception&) {
      breakCallback(std::current_exception());
    }
  }
}

#endif
