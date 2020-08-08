#ifndef BEAM_QUEUE_WRITER_HPP
#define BEAM_QUEUE_WRITER_HPP
#include "Beam/Queues/BaseQueue.hpp"
#include "Beam/Queues/Queues.hpp"

namespace Beam {

  /**
   * Interface for the write-side of a Queue.
   * @param <T> The data to write to the Queue.
   */
  template<typename T>
  class QueueWriter : public virtual BaseQueue {
    public:

      /** The type being pushed. */
      using Target = T;

      /**
       * Adds a value to the end of the Queue.
       * @param value The value to add to the end of the Queue.
       */
      virtual void Push(const Target& value) = 0;

      /**
       * Adds a value to the end of the Queue.
       * @param value The value to add to the end of the Queue.
       */
      virtual void Push(Target&& value) = 0;
  };
}

#endif
