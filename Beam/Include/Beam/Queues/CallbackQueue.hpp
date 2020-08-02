#ifndef BEAM_CALLBACK_QUEUE_HPP
#define BEAM_CALLBACK_QUEUE_HPP
#include <vector>
#include "Beam/Queues/CallbackQueueWriter.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Threading/TaskRunner.hpp"

namespace Beam {

  /** Used to translate QueueWriter pushes into callbacks. */
  class CallbackQueue : public QueueWriter<std::function<void ()>> {
    public:

      /** The type being pushed. */
      using Source = QueueWriter<std::function<void ()>>::Source;

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

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& exception) override;

      using QueueWriter<std::function<void ()>>::Break;

    private:
      bool m_isBroken;
      std::vector<std::shared_ptr<BaseQueue>> m_queues;
      Threading::TaskRunner m_tasks;
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
      [=, callback = std::forward<F>(callback)] (const T& value) {
        m_tasks.Add(
          [=, callback = &callback] {
            if(m_isBroken) {
              return;
            }
            (*callback)(value);
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
      [=] {
        if(m_isBroken) {
          queue->Break();
        } else {
          m_queues.push_back(std::move(queue));
        }
      });
    return queue;
  }

  inline void CallbackQueue::Push(const Source& value) {
    m_tasks.Add(
      [=] {
        if(m_isBroken) {
          return;
        }
        value();
      });
  }

  inline void CallbackQueue::Push(Source&& value) {
    m_tasks.Add(
      [=, value = std::move(value)] {
        if(m_isBroken) {
          return;
        }
        value();
      });
  }

  inline void CallbackQueue::Break(const std::exception_ptr& exception) {
    m_tasks.Add(
      [=] {
        if(m_isBroken) {
          return;
        }
        m_isBroken = true;
        for(auto& queue : m_queues) {
          queue->Break(exception);
        }
        m_queues.clear();
      });
  }
}

#endif
