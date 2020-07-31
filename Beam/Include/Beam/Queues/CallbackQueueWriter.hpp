#ifndef BEAM_CALLBACK_QUEUE_WRITER_HPP
#define BEAM_CALLBACK_QUEUE_WRITER_HPP
#include <functional>
#include <boost/optional/optional.hpp>
#include <boost/thread/locks.hpp>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /**
   * Used to invoke a callback when data is pushed onto this Queue.
   * @param <T> The type of data being pushed onto the Queue.
   */
  template<typename T>
  class CallbackQueueWriter final : public QueueWriter<T> {
    public:
      using Source = typename QueueWriter<T>::Source;

      /**
       * The function to call when data is pushed onto this Queue.
       * @param value The value that was pushed onto the Queue.
       */
      using CallbackFunction = std::function<void (const Source& source)>;

      /**
       * The function to call when this Queue is broken.
       * @param e Stores the reason for the break.
       */
      using BreakFunction = std::function<void (const std::exception_ptr& e)>;

      /**
       * Constructs a CallbackWriterQueue.
       * @param callback The function to call when data is pushed onto this
       *        Queue.
       */
      CallbackQueueWriter(const CallbackFunction& callback);

      /**
       * Constructs a CallbackWriterQueue.
       * @param callback The function to call when data is pushed onto this
       *        Queue.
       * @param breakCallback The function to call when this Queue is broken.
       */
      CallbackQueueWriter(const CallbackFunction& callback,
        const BreakFunction& breakCallback);

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& exception) override;

      using QueueWriter<T>::Break;
    private:
      struct Callbacks {
        CallbackFunction m_callback;
        BreakFunction m_breakCallback;
      };
      mutable Threading::RecursiveMutex m_mutex;
      bool m_isBroken;
      boost::optional<Callbacks> m_callbacks;
  };

  template<typename T>
  CallbackQueueWriter<T>::CallbackQueueWriter(const CallbackFunction& callback)
    : CallbackQueueWriter(callback, [] (const std::exception_ptr&) {}) {}

  template<typename T>
  CallbackQueueWriter<T>::CallbackQueueWriter(const CallbackFunction& callback,
    const BreakFunction& breakCallback)
    : m_isBroken(false),
      m_callbacks({callback, breakCallback}) {}

  template<typename T>
  void CallbackQueueWriter<T>::Push(const Source& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_isBroken) {
      return;
    }
    m_callbacks->m_callback(value);
  }

  template<typename T>
  void CallbackQueueWriter<T>::Push(Source&& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_isBroken) {
      return;
    }
    m_callbacks->m_callback(std::move(value));
  }

  template<typename T>
  void CallbackQueueWriter<T>::Break(const std::exception_ptr& exception) {
    {
      auto lock = boost::lock_guard(m_mutex);
      if(m_isBroken) {
        return;
      }
      m_isBroken = true;
    }
    m_callbacks->m_breakCallback(exception);
    m_callbacks.reset();
  }
}

#endif
