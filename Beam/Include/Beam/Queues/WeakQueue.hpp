#ifndef BEAM_WEAKQUEUE_HPP
#define BEAM_WEAKQUEUE_HPP
#include <memory>
#include <utility>
#include <boost/throw_exception.hpp>
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam {

  /*! \class WeakQueue
      \brief Wraps a Queue using a weak pointer.
      \tparam T The data to write to the Queue.
   */
  template<typename T>
  class WeakQueue : public QueueWriter<T> {
    public:
      typedef T Source;

      //! Constructs a WeakQueue.
      /*!
        \param queue The Queue wrap.
      */
      WeakQueue(const std::shared_ptr<QueueWriter<T>>& queue);

      virtual ~WeakQueue();

      virtual void Push(const Source& value);

      virtual void Push(Source&& value);

      virtual void Break(const std::exception_ptr& e);

      using QueueWriter<T>::Break;
    private:
      std::weak_ptr<QueueWriter<T>> m_queue;
  };

  //! Makes a WeakQueue.
  /*!
    \param queue The Queue to wrap.
    \return A WeakQueue wrapping the <i>queue</i>.
  */
  template<typename T>
  std::shared_ptr<WeakQueue<T>> MakeWeakQueue(
      const std::shared_ptr<QueueWriter<T>>& queue) {
    return std::make_shared<WeakQueue<T>>(queue);
  }

  template<typename T>
  WeakQueue<T>::WeakQueue(const std::shared_ptr<QueueWriter<T>>& queue)
      : m_queue(queue) {}

  template<typename T>
  WeakQueue<T>::~WeakQueue() {
    Break();
  }

  template<typename T>
  void WeakQueue<T>::Push(const Source& value) {
    std::shared_ptr<QueueWriter<T>> queue = m_queue.lock();
    if(queue == nullptr) {
      BOOST_THROW_EXCEPTION(PipeBrokenException());
      return;
    }
    queue->Push(value);
  }

  template<typename T>
  void WeakQueue<T>::Push(Source&& value) {
    std::shared_ptr<QueueWriter<T>> queue = m_queue.lock();
    if(queue == nullptr) {
      BOOST_THROW_EXCEPTION(PipeBrokenException());
      return;
    }
    queue->Push(std::move(value));
  }

  template<typename T>
  void WeakQueue<T>::Break(const std::exception_ptr& e) {
    std::shared_ptr<QueueWriter<T>> queue = m_queue.lock();
    if(queue == nullptr) {
      return;
    }
    queue->Break(e);
  }
}

#endif
