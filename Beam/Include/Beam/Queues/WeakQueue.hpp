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
      using Source = T;

      //! Constructs a WeakQueue.
      /*!
        \param queue The Queue wrap.
      */
      WeakQueue(std::shared_ptr<QueueWriter<T>> queue);

      virtual ~WeakQueue() override;

      virtual void Push(const Source& value) override;

      virtual void Push(Source&& value) override;

      virtual void Break(const std::exception_ptr& e) override;

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
  auto MakeWeakQueue(std::shared_ptr<QueueWriter<T>> queue) {
    return std::make_shared<WeakQueue<T>>(std::move(queue));
  }

  template<typename T>
  WeakQueue<T>::WeakQueue(std::shared_ptr<QueueWriter<T>> queue)
      : m_queue{std::move(queue)} {}

  template<typename T>
  WeakQueue<T>::~WeakQueue() {
    Break();
  }

  template<typename T>
  void WeakQueue<T>::Push(const Source& value) {
    auto queue = m_queue.lock();
    if(queue == nullptr) {
      BOOST_THROW_EXCEPTION(PipeBrokenException{});
    }
    queue->Push(value);
  }

  template<typename T>
  void WeakQueue<T>::Push(Source&& value) {
    auto queue = m_queue.lock();
    if(queue == nullptr) {
      BOOST_THROW_EXCEPTION(PipeBrokenException{});
    }
    queue->Push(std::move(value));
  }

  template<typename T>
  void WeakQueue<T>::Break(const std::exception_ptr& e) {
    auto queue = m_queue.lock();
    if(queue == nullptr) {
      return;
    }
    queue->Break(e);
  }
}

#endif
