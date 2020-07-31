#ifndef BEAM_TASK_QUEUE_HPP
#define BEAM_TASK_QUEUE_HPP
#include <iostream>
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {

  /** Used to translate Queue pushes into task functions. */
  class TaskQueue : public AbstractQueue<std::function<void ()>> {
    public:
      using Source = AbstractQueue<std::function<void ()>>::Source;
      using Target = AbstractQueue<std::function<void ()>>::Target;

      /** Constructs a TaskQueue. */
      TaskQueue() = default;

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
       * @param breakSlot The slot to call when this Queue is broken.
       * @return A Queue that translates a push into a slot invocation.
       */
      template<typename T>
      std::shared_ptr<CallbackQueueWriter<T>> GetSlot(
        const std::function<void (const T& value)>& slot,
        const std::function<void (const std::exception_ptr& e)>& breakSlot);

      /**
       * Directly emplaces a value and pops it off the stack.
       * @param value Stores the popped value.
       */
      void Emplace(Out<std::function<void ()>> value);

      bool IsEmpty() const override;

      std::function<void ()> Top() const override;

      void Pop() override;

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& exception) override;

      bool IsAvailable() const override;

      using QueueWriter<std::function<void ()>>::Break;

    private:
      Queue<std::function<void ()>> m_tasks;
      CallbackQueue m_callbacks;
  };

  /**
   * Implements a loop that runs tasks pushed onto a task Queue.
   * @param taskQueue The Queue to run tasks for.
   */
  template<typename TaskQueueType>
  void TaskLoop(TaskQueueType taskQueue) {
    try {
      while(true) {
        auto task = std::function<void ()>();
        taskQueue->Emplace(Store(task));
        task();
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
    return Routines::Spawn(
      [taskQueue = std::move(taskQueue)] {
        TaskLoop(taskQueue);
      });
  }

  /**
   * Pops off all tasks pushed onto a TaskQueue and invokes them.
   * @param tasks The TaskQueue to handle.
   */
  inline void HandleTasks(TaskQueue& tasks) {
    while(!tasks.IsEmpty()) {
      auto task = std::function<void ()>();
      tasks.Emplace(Store(task));
      task();
    }
  }

  template<typename T>
  std::shared_ptr<CallbackQueueWriter<T>> TaskQueue::GetSlot(
      const std::function<void (const T& value)>& slot) {
    return this->GetSlot<T>(slot, [] (const std::exception_ptr&) {});
  }

  template<typename T>
  std::shared_ptr<CallbackQueueWriter<T>> TaskQueue::GetSlot(
      const std::function<void (const T& value)>& slot,
      const std::function<void (const std::exception_ptr& e)>& breakSlot) {
    return m_callbacks.GetSlot<T>(
      [=] (const T& value) {
        m_tasks.Push(
          [=] () {
            slot(value);
          });
      },
      [=] (const std::exception_ptr& e) {
        m_tasks.Push(
          [=] () {
            breakSlot(e);
          });
      });
  }

  inline void TaskQueue::Emplace(Out<std::function<void ()>> value) {
    m_tasks.Emplace(Store(value));
  }

  inline bool TaskQueue::IsEmpty() const {
    return m_tasks.IsEmpty();
  }

  inline std::function<void ()> TaskQueue::Top() const {
    return m_tasks.Top();
  }

  inline void TaskQueue::Pop() {
    return m_tasks.Pop();
  }

  inline void TaskQueue::Push(const std::function<void ()>& value) {
    m_tasks.Push(value);
  }

  inline void TaskQueue::Push(std::function<void ()>&& value) {
    m_tasks.Push(std::move(value));
  }

  inline void TaskQueue::Break(const std::exception_ptr& exception) {
    m_callbacks.Break(exception);
    m_tasks.Break(exception);
  }

  inline bool TaskQueue::IsAvailable() const {
    return m_tasks.IsAvailable();
  }
}

#endif
