#ifndef BEAM_ROUTINE_TASK_QUEUE_HPP
#define BEAM_ROUTINE_TASK_QUEUE_HPP
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/TaskQueue.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam {

  /** Runs pushed tasks within a Routine. */
  class RoutineTaskQueue : public QueueWriter<std::function<void ()>> {
    public:

      /** The type being pushed. */
      using Source = QueueWriter<std::function<void ()>>::Source;

      /** Constructs a RoutineTaskQueue. */
      RoutineTaskQueue();

      ~RoutineTaskQueue();

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

      /** Waits for this queue to be broken and all tasks to complete. */
      void Wait();

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& exception) override;

      using QueueWriter<std::function<void ()>>::Break;

    private:
      TaskQueue m_tasks;
      Routines::RoutineHandler m_routine;
  };

  inline RoutineTaskQueue::RoutineTaskQueue()
    : m_routine(SpawnTaskRoutine(&m_tasks)) {}

  inline RoutineTaskQueue::~RoutineTaskQueue() {
    Break();
  }

  template<typename T>
  std::shared_ptr<CallbackQueueWriter<T>> RoutineTaskQueue::GetSlot(
      const std::function<void (const T& value)>& slot) {
    return m_tasks.GetSlot(slot);
  }

  template<typename T>
  std::shared_ptr<CallbackQueueWriter<T>> RoutineTaskQueue::GetSlot(
      const std::function<void (const T& value)>& slot,
      const std::function<void (const std::exception_ptr& e)>& breakSlot) {
    return m_tasks.GetSlot(slot, breakSlot);
  }

  inline void RoutineTaskQueue::Wait() {
    m_routine.Wait();
  }

  inline void RoutineTaskQueue::Push(const Source& value) {
    m_tasks.Push(value);
  }

  inline void RoutineTaskQueue::Push(Source&& value) {
    m_tasks.Push(std::move(value));
  }

  inline void RoutineTaskQueue::Break(const std::exception_ptr& exception) {
    m_tasks.Break(exception);
  }
}

#endif
