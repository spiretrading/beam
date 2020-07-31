#ifndef BEAM_TASK_QUEUE_HPP
#define BEAM_TASK_QUEUE_HPP
#include <iostream>
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {

  /** Used to translate queue pushes into task functions. */
  class TaskQueue : public AbstractQueue<std::function<void ()>> {
    public:
      using Source = AbstractQueue<std::function<void ()>>::Source;
      using Target = AbstractQueue<std::function<void ()>>::Target;

      /** Constructs a TaskQueue. */
      TaskQueue() = default;

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
  auto TaskQueue::GetSlot(
      const std::function<void (const T&)>& callback,
      const std::function<void (const std::exception_ptr&)>& breakCallback) {
    return GetSlotHelper<T>(callback, breakCallback);
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

  template<typename T, typename F, typename B>
  auto TaskQueue::GetSlotHelper(F&& callback, B&& breakCallback) {
    return m_callbacks.GetSlot<T>(
      [=, callback = std::forward<F>(callback)] (const T& value) {
        m_tasks.Push(
          [=, callback = &callback] {
            (*callback)(value);
          });
      },
      [=, breakCallback = std::forward<B>(breakCallback)] (
          const std::exception_ptr& e) {
        m_tasks.Push(
          [=, breakCallback = &breakCallback] () {
            (*breakCallback)(e);
          });
      });
  }
}

#endif
