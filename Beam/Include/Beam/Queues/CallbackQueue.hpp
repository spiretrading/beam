#ifndef BEAM_CALLBACK_QUEUE_HPP
#define BEAM_CALLBACK_QUEUE_HPP
#include <vector>
#include "Beam/Queues/CallbackQueueWriter.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Threading/TaskRunner.hpp"

namespace Beam {

  /** Used to translate Queue pushes into callbacks. */
  class CallbackQueue : public QueueWriter<std::function<void ()>> {
    public:

      /** The type being pushed. */
      using Source = QueueWriter<std::function<void ()>>::Source;

      /** Constructs a CallbackQueue. */
      CallbackQueue();

      ~CallbackQueue();

      /**
       * Returns a slot Queue.
       * @param slot The slot to call when a new value is pushed.
       * @return A Queue that translates a push into a slot invocation.
       */
      template<typename T>
      std::shared_ptr<CallbackQueueWriter<T>> GetSlot(
        const std::function<void (const T& value)>& slot);

      /**
       * Returns a slot Queue.
       * @param slot The slot to call when a new value is pushed.
       * @param breakSlot The slot to call when the Queue is broken.
       * @return A Queue that translates a push into a slot invocation.
       */
      template<typename T>
      std::shared_ptr<CallbackQueueWriter<T>> GetSlot(
        const std::function<void (const T& value)>& slot,
        const std::function<void (const std::exception_ptr& e)>& breakSlot);

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
    if(m_isBroken) {
      return;
    }
    m_isBroken = true;
    for(auto& queue : m_queues) {
      queue->Break();
    }
  }

  template<typename T>
  std::shared_ptr<CallbackQueueWriter<T>> CallbackQueue::GetSlot(
      const std::function<void (const T& value)>& slot) {
    return this->GetSlot<T>(slot, [] (const std::exception_ptr&) {});
  }

  template<typename T>
  std::shared_ptr<CallbackQueueWriter<T>> CallbackQueue::GetSlot(
      const std::function<void (const T& value)>& slot,
      const std::function<void (const std::exception_ptr& e)>& breakSlot) {
    auto queue = std::make_shared<CallbackQueueWriter<T>>(
      [=] (const T& value) {
        m_tasks.Add(
          [=] {
            if(m_isBroken) {
              return;
            }
            slot(value);
          });
      },
      [=] (const std::exception_ptr& e) {
        m_tasks.Add(
          [=] {
            breakSlot(e);
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
