#ifndef BEAM_CALLBACK_QUEUE_HPP
#define BEAM_CALLBACK_QUEUE_HPP
#include <concepts>
#include <mutex>
#include <vector>
#include "Beam/Queues/CallbackQueueWriter.hpp"
#include "Beam/Queues/ScopedBaseQueue.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"
#include "Beam/Threading/TaskRunner.hpp"

namespace Beam {

  /** Used to translate QueueWriter pushes into callbacks. */
  class CallbackQueue : public QueueWriter<std::function<void ()>> {
    public:

      /** The type being pushed. */
      using Target = QueueWriter<std::function<void ()>>::Target;

      /** Constructs a CallbackQueue. */
      CallbackQueue() noexcept;

      ~CallbackQueue();

      /**
       * Returns a slot.
       * @param f The callback when a new value is pushed.
       * @return A queue that translates a push into a callback.
       */
      template<typename T, std::invocable<const T&> F>
      auto get_slot(F&& f);

      /**
       * Returns a slot.
       * @param f The callback when a new value is pushed.
       * @param on_break The callback when the queue is broken.
       * @return A queue that translates a push into a callback.
       */
      template<typename T, std::invocable<const T&> F,
        std::invocable<const std::exception_ptr&> B>
      auto get_slot(F&& f, B&& on_break);

      void push(const Target& value) override;
      void push(Target&& value) override;
      void close(const std::exception_ptr& exception) override;
      using QueueWriter<std::function<void ()>>::close;

    private:
      mutable std::mutex m_mutex;
      std::exception_ptr m_exception;
      bool m_is_broken;
      std::vector<ScopedBaseQueue<>> m_queues;
      TaskRunner m_tasks;

      void rethrow();
  };

  inline CallbackQueue::CallbackQueue() noexcept
    : m_is_broken(false) {}

  inline CallbackQueue::~CallbackQueue() {
    close();
  }

  template<typename T, std::invocable<const T&> F>
  auto CallbackQueue::get_slot(F&& f) {
    auto queue = callback<T>(
      [this, f = std::make_shared<std::remove_cvref_t<F>>(
          std::forward<F>(f))] (auto&& value) {
        m_tasks.add(
          [=, value = std::forward<decltype(value)>(value)] () mutable {
            (*f)(std::forward<decltype(value)>(value));
          });
      });
    m_tasks.add([=, this] () mutable {
      if(m_is_broken) {
        queue->close(m_exception);
      } else {
        m_queues.push_back(std::move(queue));
      }
    });
    return ScopedQueueWriter(std::move(queue));
  }

  template<typename T,
    std::invocable<const T&> F, std::invocable<const std::exception_ptr&> B>
  auto CallbackQueue::get_slot(F&& f, B&& on_break) {
    auto queue = callback<T>(
      [this, f = std::make_shared<std::remove_cvref_t<F>>(
          std::forward<F>(f))] (auto&& value) {
        m_tasks.add(
          [=, value = std::forward<decltype(value)>(value)] () mutable {
            (*f)(std::forward<decltype(value)>(value));
          });
      },
      [this, on_break = std::make_shared<std::remove_cvref_t<B>>(
          std::forward<B>(on_break))] (const std::exception_ptr& e) {
        m_tasks.add([=] {
          (*on_break)(e);
        });
      });
    m_tasks.add([=, this] () mutable {
      if(m_is_broken) {
        queue->close(m_exception);
      } else {
        m_queues.push_back(std::move(queue));
      }
    });
    return ScopedQueueWriter(std::move(queue));
  }

  inline void CallbackQueue::push(const Target& value) {
    rethrow();
    m_tasks.add(value);
  }

  inline void CallbackQueue::push(Target&& value) {
    rethrow();
    m_tasks.add(std::move(value));
  }

  inline void CallbackQueue::close(const std::exception_ptr& exception) {
    {
      auto lock = std::lock_guard(m_mutex);
      if(m_exception) {
        return;
      }
      m_exception = exception;
    }
    m_tasks.add([=, this] {
      m_is_broken = true;
      for(auto& queue : m_queues) {
        queue.close(m_exception);
      }
    });
  }

  inline void CallbackQueue::rethrow() {
    auto lock = std::lock_guard(m_mutex);
    if(m_exception) {
      std::rethrow_exception(m_exception);
    }
  }
}

#endif
