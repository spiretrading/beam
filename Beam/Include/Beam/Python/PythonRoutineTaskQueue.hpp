#ifndef BEAM_PYTHON_ROUTINE_TASK_QUEUE_HPP
#define BEAM_PYTHON_ROUTINE_TASK_QUEUE_HPP
#include <memory>
#include "Beam/Queues/TaskQueue.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam::Python {

  /**
   * A RoutineTaskQueue that may be destroyed from within its own routine,
   * needed because the last reference to a Python object owning the queue can
   * be dropped by a callback running on it.
   */
  class PythonRoutineTaskQueue final :
      public QueueWriter<std::function<void ()>> {
    public:
      using Target = QueueWriter<std::function<void ()>>::Target;

      /** Constructs a PythonRoutineTaskQueue. */
      PythonRoutineTaskQueue();

      ~PythonRoutineTaskQueue() override;

      /**
       * Returns a slot.
       * @param callback The callback when a new value is pushed.
       * @return A queue that translates a push into a callback.
       */
      template<typename T, typename F>
      auto get_slot(F&& callback);

      /**
       * Returns a slot.
       * @param callback The callback when a new value is pushed.
       * @param on_break The callback when the queue is broken.
       * @return A queue that translates a push into a callback.
       */
      template<typename T, typename F, typename B>
      auto get_slot(F&& callback, B&& on_break);

      /** Waits for this queue to be broken and all its tasks to complete. */
      void wait();

      void push(const Target& value) override;
      void push(Target&& value) override;
      void close(const std::exception_ptr& e) override;
      using QueueWriter<std::function<void ()>>::close;

    private:
      std::shared_ptr<TaskQueue> m_tasks;
      RoutineHandler m_routine;
  };

  inline PythonRoutineTaskQueue::PythonRoutineTaskQueue()
    : m_tasks(std::make_shared<TaskQueue>()),
      m_routine(spawn_loop(m_tasks)) {}

  inline PythonRoutineTaskQueue::~PythonRoutineTaskQueue() {
    m_tasks->close();
    if(get_current_routine().get_id() == m_routine.get_id()) {
      m_routine.detach();
    }
  }

  template<typename T, typename F>
  auto PythonRoutineTaskQueue::get_slot(F&& callback) {
    return m_tasks->get_slot<T>(std::forward<F>(callback));
  }

  template<typename T, typename F, typename B>
  auto PythonRoutineTaskQueue::get_slot(F&& callback, B&& on_break) {
    return m_tasks->get_slot<T>(
      std::forward<F>(callback), std::forward<B>(on_break));
  }

  inline void PythonRoutineTaskQueue::wait() {
    m_routine.wait();
  }

  inline void PythonRoutineTaskQueue::push(const Target& value) {
    m_tasks->push(value);
  }

  inline void PythonRoutineTaskQueue::push(Target&& value) {
    m_tasks->push(std::move(value));
  }

  inline void PythonRoutineTaskQueue::close(const std::exception_ptr& e) {
    m_tasks->close(e);
  }
}

#endif
