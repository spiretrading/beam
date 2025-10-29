#ifndef BEAM_CALLBACK_QUEUE_WRITER_HPP
#define BEAM_CALLBACK_QUEUE_WRITER_HPP
#include <functional>
#include <iostream>
#include <memory>
#include <boost/optional/optional.hpp>
#include <boost/thread/locks.hpp>
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {

  /**
   * Used to invoke a callback when data is pushed onto this queue.
   * @tparam T The type of data being pushed onto the queue.
   * @tparam F The type called when data is pushed onto this queue.
   * @tparam B The type called when this queue is broken.
   */
  template<typename T,
    std::invocable<const typename QueueWriter<T>::Target&> F =
      std::function<void (const typename QueueWriter<T>::Target&)>,
    std::invocable<const std::exception_ptr&> B =
      std::function<void (const std::exception_ptr&)>>
  class CallbackQueueWriter final : public QueueWriter<T> {
    public:
      using Target = typename QueueWriter<T>::Target;

      /**
       * The function to call when data is pushed onto this queue.
       * @param value The value that was pushed onto the queue.
       */
      using Callback = F;

      /**
       * The function to call when this queue is broken.
       * @param e Stores the reason for the break.
       */
      using BreakCallback = B;

      /**
       * Constructs a CallbackWriterQueue.
       * @param f The function to call when data is pushed onto this queue.
       */
      explicit CallbackQueueWriter(Callback f);

      /**
       * Constructs a CallbackWriterQueue.
       * @param f The function to call when data is pushed onto this queue.
       * @param on_break The function to call when this queue is broken.
       */
      CallbackQueueWriter(Callback f, BreakCallback on_break);

      void push(const Target& value) override;
      void push(Target&& value) override;
      void close(const std::exception_ptr& e) override;
      using QueueWriter<T>::close;

    private:
      struct Callbacks {
        Callback m_callback;
        BreakCallback m_on_break;
      };
      mutable RecursiveMutex m_mutex;
      std::exception_ptr m_exception;
      boost::optional<Callbacks> m_callbacks;
  };

  /**
   * Makes a CallbackQueueWriter for a given callback and break callback.
   * @param f The function to call when data is pushed onto this queue.
   * @param on_break The function to call when this queue is broken.
   */
  template<typename T,
    std::invocable<const typename QueueWriter<T>::Target&> F,
    std::invocable<const std::exception_ptr&> B>
  auto callback(F&& f, B&& on_break) {
    return std::make_shared<CallbackQueueWriter<
      T, std::remove_cvref_t<F>, std::remove_cvref_t<B>>>(
        std::forward<F>(f), std::forward<B>(on_break));
  }

  /**
   * Makes a CallbackQueueWriter for a given callback and break callback.
   * @param f The function to call when data is pushed onto this queue.
   */
  template<typename T,
    std::invocable<const typename QueueWriter<T>::Target&> F>
  auto callback(F&& f) {
    return callback<T>(std::forward<F>(f), [] (const std::exception_ptr&) {});
  }

  template<typename T,
    std::invocable<const typename QueueWriter<T>::Target&> F,
    std::invocable<const std::exception_ptr&> B>
  CallbackQueueWriter<T, F, B>::CallbackQueueWriter(Callback f)
    : CallbackQueueWriter(std::move(f), [] (const std::exception_ptr&) {}) {}

  template<typename T,
    std::invocable<const typename QueueWriter<T>::Target&> F,
    std::invocable<const std::exception_ptr&> B>
  CallbackQueueWriter<T, F, B>::CallbackQueueWriter(
    Callback f, BreakCallback on_break)
    : m_callbacks(boost::in_place_init, std::move(f), std::move(on_break)) {}

  template<typename T,
    std::invocable<const typename QueueWriter<T>::Target&> F,
    std::invocable<const std::exception_ptr&> B>
  void CallbackQueueWriter<T, F, B>::push(const Target& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_exception) {
      std::rethrow_exception(m_exception);
    }
    m_callbacks->m_callback(value);
  }

  template<typename T,
    std::invocable<const typename QueueWriter<T>::Target&> F,
    std::invocable<const std::exception_ptr&> B>
  void CallbackQueueWriter<T, F, B>::push(Target&& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_exception) {
      std::rethrow_exception(m_exception);
    }
    m_callbacks->m_callback(std::move(value));
  }

  template<typename T,
    std::invocable<const typename QueueWriter<T>::Target&> F,
    std::invocable<const std::exception_ptr&> B>
  void CallbackQueueWriter<T, F, B>::close(const std::exception_ptr& e) {
    {
      auto lock = boost::lock_guard(m_mutex);
      if(m_exception) {
        return;
      }
      m_exception = e;
    }
    try {
      m_callbacks->m_on_break(e);
    } catch(const std::exception&) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
    }
    m_callbacks.reset();
  }
}

#endif
