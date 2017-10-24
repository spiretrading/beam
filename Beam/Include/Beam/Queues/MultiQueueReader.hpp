#ifndef BEAM_MULTI_QUEUE_READER_HPP
#define BEAM_MULTI_QUEUE_READER_HPP
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/Queue.hpp"

namespace Beam {

  /*! \class MultiQueueReader
      \brief Consolidates multiple QueueReaders together.
      \tparam T The type to read from the queue.
   */
  template<typename T>
  class MultiQueueReader : public AbstractQueue<T> {
    public:

      //! The type to read from the queue.
      using Target = T;

      //! The type to write to the queue.
      using Source = T;

      //! Constructs a MultiQueueReader.
      MultiQueueReader() = default;

      //! Returns a QueueWriter for pushing values onto this queue.
      std::shared_ptr<QueueWriter<Target>> GetWriter();

      virtual bool IsEmpty() const override final;

      virtual Target Top() const override final;

      virtual void Pop() override final;

      virtual void Break(const std::exception_ptr& e) override final;

      virtual void Push(const Source& value) override final;

      virtual void Push(Source&& value) override final;

    protected:
      virtual bool IsAvailable() const override final;

    private:
      Queue<Target> m_queue;
      CallbackQueue m_callbacks;
  };

  template<typename T>
  std::shared_ptr<QueueWriter<typename MultiQueueReader<T>::Target>>
      MultiQueueReader<T>::GetWriter() {
    return m_callbacks.GetSlot<T>(
      [=] (const T& value) {
        m_queue.Push(value);
      });
  }

  template<typename T>
  bool MultiQueueReader<T>::IsEmpty() const {
    return m_queue.IsEmpty();
  }

  template<typename T>
  typename MultiQueueReader<T>::Target MultiQueueReader<T>::Top() const {
    return m_queue.Top();
  }

  template<typename T>
  void MultiQueueReader<T>::Pop() {
    return m_queue.Pop();
  }

  template<typename T>
  void MultiQueueReader<T>::Break(const std::exception_ptr& e) {
    m_callbacks.Break();
    m_queue.Break();
  }

  template<typename T>
  void MultiQueueReader<T>::Push(const Source& value) {
    m_queue.Push(value);
  }

  template<typename T>
  void MultiQueueReader<T>::Push(Source&& value) {
    m_queue.Push(std::move(value));
  }

  template<typename T>
  bool MultiQueueReader<T>::IsAvailable() const {
    return m_queue.IsAvailable();
  }
}

#endif
