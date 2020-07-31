#ifndef BEAM_CALLBACK_QUEUE_WRITER_HPP
#define BEAM_CALLBACK_QUEUE_WRITER_HPP
#include <functional>
#include <memory>
#include <boost/optional/optional.hpp>
#include <boost/thread/locks.hpp>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /**
   * Used to invoke a callback when data is pushed onto this queue.
   * @param <T> The type of data being pushed onto the queue.
   * @param <C> The type called when data is pushed onto this queue.
   * @param <B> The type called when this queue is broken.
   */
  template<typename T,
    typename C = std::function<void (const typename QueueWriter<T>::Source&)>,
    typename B = std::function<void (const std::exception_ptr&)>>
  class CallbackQueueWriter final : public QueueWriter<T> {
    public:
      using Source = typename QueueWriter<T>::Source;

      /**
       * The function to call when data is pushed onto this queue.
       * @param value The value that was pushed onto the queue.
       */
      using Callback = C;

      /**
       * The function to call when this queue is broken.
       * @param e Stores the reason for the break.
       */
      using BreakCallback = B;

      /**
       * Constructs a CallbackWriterQueue.
       * @param callback The function to call when data is pushed onto this
       *        queue.
       */
      CallbackQueueWriter(Callback callback);

      /**
       * Constructs a CallbackWriterQueue.
       * @param callback The function to call when data is pushed onto this
       *        queue.
       * @param breakCallback The function to call when this queue is broken.
       */
      CallbackQueueWriter(Callback callback, BreakCallback breakCallback);

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<T>::Break;

    private:
      struct Callbacks {
        Callback m_callback;
        BreakCallback m_breakCallback;
      };
      mutable Threading::RecursiveMutex m_mutex;
      bool m_isBroken;
      boost::optional<Callbacks> m_callbacks;
  };

  /**
   * Makes a CallbackQueueWriter for a given callback and break callback.
   * @param callback The function to call when data is pushed onto this
   *        queue.
   * @param breakCallback The function to call when this queue is broken.
   */
  template<typename T, typename C, typename B>
  auto MakeCallbackQueueWriter(C&& callback, B&& breakCallback) {
    return std::make_shared<
      CallbackQueueWriter<T, std::decay_t<C>, std::decay_t<B>>>(
      std::forward<C>(callback), std::forward<B>(breakCallback));
  }

  /**
   * Makes a CallbackQueueWriter for a given callback and break callback.
   * @param callback The function to call when data is pushed onto this
   *        queue.
   */
  template<typename T, typename C>
  auto MakeCallbackQueueWriter(C&& callback) {
    return MakeCallbackQueueWriter(std::forward<C>(callback),
      [] (const std::exception_ptr&) {});
  }

  template<typename T, typename C, typename B>
  CallbackQueueWriter<T, C, B>::CallbackQueueWriter(Callback callback)
    : CallbackQueueWriter(std::move(callback),
        [] (const std::exception_ptr&) {}) {}

  template<typename T, typename C, typename B>
  CallbackQueueWriter<T, C, B>::CallbackQueueWriter(Callback callback,
    BreakCallback breakCallback)
    : m_isBroken(false),
      m_callbacks({std::move(callback), std::move(breakCallback)}) {}

  template<typename T, typename C, typename B>
  void CallbackQueueWriter<T, C, B>::Push(const Source& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_isBroken) {
      return;
    }
    m_callbacks->m_callback(value);
  }

  template<typename T, typename C, typename B>
  void CallbackQueueWriter<T, C, B>::Push(Source&& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_isBroken) {
      return;
    }
    m_callbacks->m_callback(std::move(value));
  }

  template<typename T, typename C, typename B>
  void CallbackQueueWriter<T, C, B>::Break(const std::exception_ptr& e) {
    {
      auto lock = boost::lock_guard(m_mutex);
      if(m_isBroken) {
        return;
      }
      m_isBroken = true;
    }
    m_callbacks->m_breakCallback(e);
    m_callbacks.reset();
  }
}

#endif
