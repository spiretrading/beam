#ifndef BEAM_TASK_QUEUE_HPP
#define BEAM_TASK_QUEUE_HPP
#include <atomic>
#include <iostream>
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Routines/Routine.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {

  /** Used to translate queue pushes into task functions. */
  class TaskQueue : public AbstractQueue<std::function<void ()>> {
    public:
      using Target = AbstractQueue<std::function<void ()>>::Target;
      using Source = AbstractQueue<std::function<void ()>>::Source;

      /** Constructs a TaskQueue. */
      TaskQueue();

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
       * @return A queue that translates a push into a callback.
       */
      template<typename T>
      auto GetSlot(const std::function<void (const T&)>& callback);

      /**
       * Returns a slot.
       * @param callback The callback when a new value is pushed.
       * @param breakCallback The callback when the queue is broken.
       * @return A queue that translates a push into a callback.
       */
      template<typename T, typename F, typename B>
      auto GetSlot(F&& callback, B&& breakCallback);

      /**
       * Returns a slot.
       * @param callback The callback when a new value is pushed.
       * @param breakCallback The callback when the queue is broken.
       * @return A queue that translates a push into a callback.
       */
      template<typename T>
      auto GetSlot(const std::function<void (const T&)>& callback,
        const std::function<void (const std::exception_ptr&)>& breakCallback);

      Source Pop() override;

      boost::optional<Source> TryPop() override;

      void Push(const Target& value) override;

      void Push(Target&& value) override;

      void Break(const std::exception_ptr& exception) override;

      using AbstractQueue<std::function<void ()>>::Break;

    private:
      std::atomic_bool m_isBroken;
      Queue<Source> m_tasks;
      CallbackQueue m_callbacks;

      template<typename T, typename F, typename B>
      auto GetSlotHelper(F&& callback, B&& breakCallback);
  };

  /**
   * Implements a loop that runs tasks pushed onto a task Queue.
   * @param taskQueue The Queue to run tasks for.
   */
  template<typename TaskQueueType>
  void TaskLoop(TaskQueueType taskQueue) {
    try {
      while(true) {
        taskQueue->Pop()();
      }
    } catch(const PipeBrokenException&) {
      return;
    } catch(const std::exception&) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
    }
  }

  /**
   * Spawns a Routine that executes tasks.
   * @param taskQueue The Queue to read the tasks from.
   * @return The spawned Routine's Id.
   */
  template<typename TaskQueueType>
  Routines::Routine::Id SpawnTaskRoutine(TaskQueueType taskQueue) {
    return Routines::Spawn([taskQueue = std::move(taskQueue)] {
      TaskLoop(taskQueue);
    });
  }

  /**
   * Pops off all tasks pushed onto a TaskQueue and invokes them.
   * @param tasks The TaskQueue to handle.
   */
  inline void HandleTasks(TaskQueue& tasks) {
    while(auto task = tasks.TryPop()) {
      (*task)();
    }
  }

  inline TaskQueue::TaskQueue()
    : m_isBroken(false) {}

  template<typename T, typename F>
  auto TaskQueue::GetSlot(F&& callback) {
    return GetSlot<T>(std::forward<F>(callback),
      [] (const std::exception_ptr&) {});
  }

  template<typename T>
  auto TaskQueue::GetSlot(
      const std::function<void (const T&)>& callback) {
    return GetSlot(callback, std::function<void (const std::exception_ptr&)>(
      [] (const std::exception_ptr&) {}));
  }

  template<typename T, typename F, typename B>
  auto TaskQueue::GetSlot(F&& callback, B&& breakCallback) {
    return GetSlotHelper<T>(std::forward<F>(callback),
      std::forward<B>(breakCallback));
  }

  template<typename T>
  auto TaskQueue::GetSlot(const std::function<void (const T&)>& callback,
      const std::function<void (const std::exception_ptr&)>& breakCallback) {
    return GetSlotHelper<T>(callback, breakCallback);
  }

  inline TaskQueue::Source TaskQueue::Pop() {
    return m_tasks.Pop();
  }

  inline boost::optional<TaskQueue::Source> TaskQueue::TryPop() {
    return m_tasks.TryPop();
  }

  inline void TaskQueue::Push(const Target& value) {
    m_tasks.Push(value);
  }

  inline void TaskQueue::Push(Target&& value) {
    m_tasks.Push(std::move(value));
  }

  inline void TaskQueue::Break(const std::exception_ptr& exception) {
    if(!m_isBroken.exchange(true)) {
      m_callbacks.Break(exception);
      Push([=, this] {
        m_tasks.Break(exception);
      });
    }
  }

  template<typename T, typename F, typename B>
  auto TaskQueue::GetSlotHelper(F&& callback, B&& breakCallback) {
    return m_callbacks.GetSlot<T>(
      [this, callback = std::make_shared<std::remove_reference_t<F>>(
          std::forward<F>(callback))] (const T& value) {
        m_tasks.Push([=, this] {
          (*callback)(value);
        });
      },
      [this, breakCallback = std::make_shared<std::remove_reference_t<B>>(
          std::forward<B>(breakCallback))] (const std::exception_ptr& e) {
        m_tasks.Push([=, this] () {
          (*breakCallback)(e);
        });
      });
  }
}

#endif
