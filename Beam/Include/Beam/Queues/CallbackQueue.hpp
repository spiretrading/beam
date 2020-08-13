#ifndef BEAM_CALLBACK_QUEUE_HPP
#define BEAM_CALLBACK_QUEUE_HPP
#include <vector>
#include "Beam/Queues/CallbackQueueWriter.hpp"
#include "Beam/Queues/Queues.hpp"
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
      CallbackQueue();

      ~CallbackQueue();

      /**
       * Returns a slot.
       * @param callback The callback when a new value is pushed.
       * @return A queue that translates a push into a callback.
       */
      template<typename T, typename F>
      auto GetSlot(F&& callback);

      /**
       * Returns a slot.
       * @param callback The callback when a new value is pushed.
       * @param breakCallback The callback when the queue is broken.
       * @return A queue that translates a push into a callback.
       */
      template<typename T, typename F, typename B>
      auto GetSlot(F&& callback, B&& breakCallback);

      void Push(const Target& value) override;

      void Push(Target&& value) override;

      void Break(const std::exception_ptr& exception) override;

      using QueueWriter<std::function<void ()>>::Break;

    private:
      mutable std::mutex m_mutex;
      std::exception_ptr m_exception;
      bool m_isBroken;
      std::vector<ScopedBaseQueue<>> m_queues;
      Threading::TaskRunner m_tasks;

      void Rethrow();
  };

  inline CallbackQueue::CallbackQueue()
    : m_isBroken(false) {}

  inline CallbackQueue::~CallbackQueue() {
    Break();
  }

  template<typename T, typename F>
  auto CallbackQueue::GetSlot(F&& callback) {
    return this->GetSlot<T>(std::forward<F>(callback),
      [] (const std::exception_ptr&) {});
  }

  template<typename T, typename F, typename B>
  auto CallbackQueue::GetSlot(F&& callback, B&& breakCallback) {
    auto queue = MakeCallbackQueueWriter<T>(
      [=, callback = std::forward<F>(callback)] (auto&& value) {
        m_tasks.Add(
          [callback = &callback,
              value = std::forward<decltype(value)>(value)] () mutable {
            (*callback)(std::forward<decltype(value)>(value));
          });
      },
      [=, breakCallback = std::forward<B>(breakCallback)] (
          const std::exception_ptr& e) {
        m_tasks.Add(
          [=, breakCallback = &breakCallback] {
            (*breakCallback)(e);
          });
      });
    m_tasks.Add(
      [=] () mutable {
        if(m_isBroken) {
          queue->Break(m_exception);
        } else {
          m_queues.push_back(std::move(queue));
        }
      });
    return ScopedQueueWriter(std::move(queue));
  }

  inline void CallbackQueue::Push(const Target& value) {
    Rethrow();
    m_tasks.Add(value);
  }

  inline void CallbackQueue::Push(Target&& value) {
    Rethrow();
    m_tasks.Add(std::move(value));
  }

  inline void CallbackQueue::Break(const std::exception_ptr& exception) {
    {
      auto lock = std::lock_guard(m_mutex);
      if(m_exception) {
        return;
      }
      m_exception = exception;
    }
    m_tasks.Add(
      [=] {
        m_isBroken = true;
        for(auto& queue : m_queues) {
          queue.Break(m_exception);
        }
      });
  }

  inline void CallbackQueue::Rethrow() {
    auto lock = std::lock_guard(m_mutex);
    if(m_exception) {
      std::rethrow_exception(m_exception);
    }
  }
}

#endif
