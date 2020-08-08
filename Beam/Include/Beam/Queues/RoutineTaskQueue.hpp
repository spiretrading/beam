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
      using Target = QueueWriter<std::function<void ()>>::Target;

      /** Constructs a RoutineTaskQueue. */
      RoutineTaskQueue();

      ~RoutineTaskQueue();

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

      /** Waits for this queue to be broken and all tasks to complete. */
      void Wait();

      void Push(const Target& value) override;

      void Push(Target&& value) override;

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

  template<typename T, typename F>
  auto RoutineTaskQueue::GetSlot(F&& callback) {
    return m_tasks.GetSlot<T>(std::forward<F>(callback));
  }

  template<typename T>
  auto RoutineTaskQueue::GetSlot(
      const std::function<void (const T&)>& callback) {
    return m_tasks.GetSlot(callback);
  }

  template<typename T, typename F, typename B>
  auto RoutineTaskQueue::GetSlot(F&& callback, B&& breakCallback) {
    return m_tasks.GetSlot<T>(std::forward<F>(callback),
      std::forward<B>(breakCallback));
  }

  template<typename T>
  auto RoutineTaskQueue::GetSlot(
      const std::function<void (const T&)>& callback,
      const std::function<void (const std::exception_ptr&)>& breakCallback) {
    return m_tasks.GetSlot(callback, breakCallback);
  }

  inline void RoutineTaskQueue::Wait() {
    m_routine.Wait();
  }

  inline void RoutineTaskQueue::Push(const Target& value) {
    m_tasks.Push(value);
  }

  inline void RoutineTaskQueue::Push(Target&& value) {
    m_tasks.Push(std::move(value));
  }

  inline void RoutineTaskQueue::Break(const std::exception_ptr& exception) {
    m_tasks.Break(exception);
  }
}

#endif
