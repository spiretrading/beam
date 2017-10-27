#ifndef BEAM_QUEUEREADER_HPP
#define BEAM_QUEUEREADER_HPP
#include "Beam/Queues/BaseQueue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Threading/Waitable.hpp"

namespace Beam {

  /*! \class QueueReader
      \brief Interface for the read-only side of a Queue.
      \tparam T The data to read from the Queue.
   */
  template<typename T>
  class QueueReader : public virtual BaseQueue, public Threading::Waitable {
    public:

      //! The type being read.
      using Target = T;

      virtual ~QueueReader() = default;

      //! Returns <code>true</code> iff the Queue is empty.
      virtual bool IsEmpty() const = 0;

      //! Returns the top value in the Queue.
      virtual Target Top() const = 0;

      //! Removes the top value in the Queue.
      virtual void Pop() = 0;

      //! Blocks until a value is available to be popped.
      void Wait() const;

    protected:
      template<typename U>
      static bool IsAvailable(const QueueReader<U>& queue);
  };

  //! Flushes the contents of a QueueReader into an iterator.
  /*!
    \param queue The QueueReader to flush.
    \param destination An iterator to the first position to flush the
           <i>queue<i> to.
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
  void Monitor(const Queue& queue, const F1& valueCallback,
      const F2& breakCallback) {
    try {
      while(true) {
        valueCallback(queue->Top());
        queue->Pop();
      }
    } catch(const std::exception&) {
      breakCallback(std::current_exception());
    }
  }

  template<typename T>
  void QueueReader<T>::Wait() const {
    boost::unique_lock<boost::mutex> lock{GetMutex()};
    Threading::Waitable::Wait(lock);
  }

  template<typename T>
  template<typename U>
  bool QueueReader<T>::IsAvailable(const QueueReader<U>& queue) {
    return Threading::Waitable::IsAvailable(
      static_cast<const Threading::Waitable&>(queue));
  }
}

#endif
