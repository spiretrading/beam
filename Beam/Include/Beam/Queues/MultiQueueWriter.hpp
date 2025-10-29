#ifndef BEAM_MULTI_QUEUE_WRITER_HPP
#define BEAM_MULTI_QUEUE_WRITER_HPP
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queue.hpp"

namespace Beam {

  /**
   * Implements a multiple writer, single reader AbstractQueue.
   * @tparam T The type to push and pop.
   */
  template<typename T>
  class MultiQueueWriter final : public AbstractQueue<T> {
    public:
      using Target = typename AbstractQueue<T>::Target;
      using Source = typename AbstractQueue<T>::Source;

      /** Constructs a MultiQueueWriter. */
      MultiQueueWriter() = default;

      /** Returns a QueueWriter for pushing values onto this queue. */
      auto get_writer();

      Source pop() override;
      boost::optional<Source> try_pop() override;
      void close(const std::exception_ptr& e) override;
      void push(const Target& value) override;
      void push(Target&& value) override;
      using AbstractQueue<T>::close;

    private:
      Queue<Source> m_queue;
      CallbackQueue m_callbacks;
  };

  template<typename T>
  auto MultiQueueWriter<T>::get_writer() {
    return m_callbacks.get_slot<Source>([this] (auto&& value) {
      m_queue.push(std::forward<decltype(value)>(value));
    });
  }

  template<typename T>
  typename MultiQueueWriter<T>::Source MultiQueueWriter<T>::pop()  {
    return m_queue.pop();
  }

  template<typename T>
  boost::optional<typename MultiQueueWriter<T>::Source>
      MultiQueueWriter<T>::try_pop()  {
    return m_queue.try_pop();
  }

  template<typename T>
  void MultiQueueWriter<T>::close(const std::exception_ptr& e) {
    m_callbacks.close(e);
    m_queue.close(e);
  }

  template<typename T>
  void MultiQueueWriter<T>::push(const Target& value) {
    m_queue.push(value);
  }

  template<typename T>
  void MultiQueueWriter<T>::push(Target&& value) {
    m_queue.push(std::move(value));
  }
}

#endif
