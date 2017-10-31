#ifndef BEAM_TASKQUEUE_HPP
#define BEAM_TASKQUEUE_HPP
#include <iostream>
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {

  /*! \class TaskQueue
      \brief Used to translate Queue pushes into task functions.
   */
  class TaskQueue : public AbstractQueue<std::function<void ()>> {
    public:
      using Target = QueueReader<std::function<void ()>>::Target;

      //! Constructs a TaskQueue.
      TaskQueue() = default;

      virtual ~TaskQueue();

      //! Returns a slot Queue.
      /*!
        \param slot The slot to call when a new value is pushed.
        \return A Queue that translates a push into a slot invocation.
      */
      template<typename T>
      std::shared_ptr<CallbackWriterQueue<T>> GetSlot(
        const std::function<void (const T& value)>& slot);

      //! Returns a slot Queue.
      /*!
        \param slot The slot to call when a new value is pushed.
        \param breakSlot The slot to call when this Queue is broken.
        \return A Queue that translates a push into a slot invocation.
      */
      template<typename T>
      std::shared_ptr<CallbackWriterQueue<T>> GetSlot(
        const std::function<void (const T& value)>& slot,
        const std::function<void (const std::exception_ptr& e)>& breakSlot);

      virtual bool IsEmpty() const;

      virtual void Wait() const;

      virtual std::function<void ()> Top() const;

      virtual void Emplace(Out<std::function<void ()>> value);

      virtual void Pop();

      virtual void Push(const Source& value);

      virtual void Push(Source&& value);

      virtual void Break(const std::exception_ptr& exception);

      using QueueWriter<std::function<void ()>>::Break;

      virtual bool IsAvailable() const;

    private:
      Queue<std::function<void ()>> m_tasks;
      CallbackQueue m_callbacks;
  };

  //! Implements a loop that runs tasks pushed onto a task Queue.
  /*!
    \param taskQueue The Queue to run tasks for.
  */
  template<typename TaskQueueType>
  void TaskLoop(TaskQueueType taskQueue) {
    try {
      while(true) {
        std::function<void ()> task;
        taskQueue->Emplace(Store(task));
        task();
      }
    } catch(const PipeBrokenException&) {
      return;
    } catch(const std::exception&) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
    }
  }

  //! Spawns a Routine that executes tasks.
  /*!
    \param taskQueue The Queue to read the tasks from.
    \return The spawned Routine's Id.
  */
  template<typename TaskQueueType>
  Routines::Routine::Id SpawnTaskRoutine(TaskQueueType taskQueue) {
    return Routines::Spawn(
      [=] {
        TaskLoop(taskQueue);
      });
  }

  //! Pops off all tasks pushed onto a TaskQueue and invokes them.
  /*!
    \param tasks The TaskQueue to handle.
  */
  inline void HandleTasks(TaskQueue& tasks) {
    while(!tasks.IsEmpty()) {
      std::function<void ()> task;
      tasks.Emplace(Store(task));
      task();
    }
  }

  inline TaskQueue::~TaskQueue() {
    Break();
  }

  template<typename T>
  std::shared_ptr<CallbackWriterQueue<T>> TaskQueue::GetSlot(
      const std::function<void (const T& value)>& slot) {
    return this->GetSlot<T>(slot, [] (const std::exception_ptr&) {});
  }

  template<typename T>
  std::shared_ptr<CallbackWriterQueue<T>> TaskQueue::GetSlot(
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

  inline bool TaskQueue::IsEmpty() const {
    return m_tasks.IsEmpty();
  }

  inline void TaskQueue::Wait() const {
    return m_tasks.Wait();
  }

  inline std::function<void ()> TaskQueue::Top() const {
    return m_tasks.Top();
  }

  inline void TaskQueue::Emplace(Out<std::function<void ()>> value) {
    m_tasks.Emplace(Store(value));
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
