#ifndef BEAM_BASE_QUEUE_HPP
#define BEAM_BASE_QUEUE_HPP
#include <exception>
#include "Beam/Queues/PipeBrokenException.hpp"

namespace Beam {

  /** Base class for a Queue. */
  class BaseQueue {
    public:
      virtual ~BaseQueue() = default;

      /** Breaks the Queue, indicating no further values will be published. */
      virtual void close();

      /**
       * Breaks the Queue, indicating no further values will be published.
       * @param e The reason why the Queue was broken.
       */
      virtual void close(const std::exception_ptr& e) = 0;

      /**
       * Breaks the Queue, indicating no further values will be published.
       * @param e The reason why the Queue was broken.
       */
      template<typename E>
      void close(const E& e);

    protected:

      /** Constructs a BaseQueue. */
      BaseQueue() = default;

    private:
      BaseQueue(const BaseQueue&) = delete;
      BaseQueue& operator =(const BaseQueue&) = delete;
  };

  inline void BaseQueue::close() {
    close(PipeBrokenException());
  }

  template<typename E>
  void BaseQueue::close(const E& e) {
    close(std::make_exception_ptr(e));
  }
}

#endif
