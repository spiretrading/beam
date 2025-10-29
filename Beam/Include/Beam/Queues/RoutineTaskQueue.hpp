#ifndef BEAM_ROUTINE_TASK_QUEUE_HPP
#define BEAM_ROUTINE_TASK_QUEUE_HPP
#include <concepts>
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
      template<typename T, std::invocable<const T&> F>
      auto get_slot(F&& callback);

      /**
       * Returns a slot.
       * @param callback The callback when a new value is pushed.
       * @return A queue that translates a push into a callback.
       */
      template<typename T>
      auto get_slot(const std::function<void (const T&)>& callback);

      /**
       * Returns a slot.
       * @param callback The callback when a new value is pushed.
       * @param on_break The callback when the queue is broken.
       * @return A queue that translates a push into a callback.
       */
      template<typename T, std::invocable<const T&> F,
        std::invocable<const std::exception_ptr&> B>
      auto get_slot(F&& callback, B&& on_break);

      /**
       * Returns a slot.
       * @param callback The callback when a new value is pushed.
       * @param on_break The callback when the queue is broken.
       * @return A queue that translates a push into a callback.
       */
      template<typename T>
      auto get_slot(const std::function<void (const T&)>& callback,
        const std::function<void (const std::exception_ptr&)>& on_break);

      /** Waits for this queue to be broken and all tasks to complete. */
      void wait();

      void push(const Target& value) override;
      void push(Target&& value) override;
      void close(const std::exception_ptr& exception) override;
      using QueueWriter<std::function<void ()>>::close;

    private:
      TaskQueue m_tasks;
      RoutineHandler m_routine;
  };

  inline RoutineTaskQueue::RoutineTaskQueue()
    : m_routine(spawn_loop(&m_tasks)) {}

  inline RoutineTaskQueue::~RoutineTaskQueue() {
    close();
  }

  template<typename T, std::invocable<const T&> F>
  auto RoutineTaskQueue::get_slot(F&& callback) {
    return m_tasks.get_slot<T>(std::forward<F>(callback));
  }

  template<typename T>
  auto RoutineTaskQueue::get_slot(
      const std::function<void (const T&)>& callback) {
    return m_tasks.get_slot(callback);
  }

  template<typename T, std::invocable<const T&> F,
    std::invocable<const std::exception_ptr&> B>
  auto RoutineTaskQueue::get_slot(F&& callback, B&& on_break) {
    return m_tasks.get_slot<T>(
      std::forward<F>(callback), std::forward<B>(on_break));
  }

  template<typename T>
  auto RoutineTaskQueue::get_slot(
      const std::function<void (const T&)>& callback,
      const std::function<void (const std::exception_ptr&)>& on_break) {
    return m_tasks.get_slot(callback, on_break);
  }

  inline void RoutineTaskQueue::wait() {
    m_routine.wait();
  }

  inline void RoutineTaskQueue::push(const Target& value) {
    m_tasks.push(value);
  }

  inline void RoutineTaskQueue::push(Target&& value) {
    m_tasks.push(std::move(value));
  }

  inline void RoutineTaskQueue::close(const std::exception_ptr& exception) {
    m_tasks.close(exception);
  }
}

#endif
