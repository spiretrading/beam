#ifndef BEAM_TASK_QUEUE_HPP
#define BEAM_TASK_QUEUE_HPP
#include <concepts>
#include <atomic>
#include <iostream>
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {

  /** Used to translate queue pushes into task functions. */
  class TaskQueue : public AbstractQueue<std::function<void ()>> {
    public:
      using Target = AbstractQueue<std::function<void ()>>::Target;
      using Source = AbstractQueue<std::function<void ()>>::Source;

      /** Constructs a TaskQueue. */
      TaskQueue() noexcept;

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

      Source pop() override;
      boost::optional<Source> try_pop() override;
      void push(const Target& value) override;
      void push(Target&& value) override;
      void close(const std::exception_ptr& exception) override;
      using AbstractQueue<std::function<void ()>>::close;

    private:
      std::atomic_bool m_is_broken;
      Queue<Source> m_tasks;
      CallbackQueue m_callbacks;

      template<typename T, typename F, typename B>
      auto get_slot_helper(F&& callback, B&& on_break);
  };

  /**
   * Implements a loop that runs tasks pushed onto a task Queue.
   * @param queue The Queue to run tasks for.
   */
  template<typename Q>
  void loop(Q queue) requires std::derived_from<
      dereference_t<Q>, QueueReader<typename dereference_t<Q>::Source>> &&
        std::invocable<typename dereference_t<Q>::Source> {
    try {
      while(true) {
        queue->pop()();
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
  template<typename Q> requires std::derived_from<
      dereference_t<Q>, QueueReader<typename dereference_t<Q>::Source>> &&
        std::invocable<typename dereference_t<Q>::Source>
  Routine::Id spawn_loop(Q queue) {
    return spawn([queue = std::move(queue)] {
      loop(queue);
    });
  }

  /**
   * Pops off all tasks pushed onto a TaskQueue and invokes them.
   * @param tasks The TaskQueue to handle.
   */
  inline void flush(TaskQueue& tasks) {
    while(auto task = tasks.try_pop()) {
      (*task)();
    }
  }

  inline TaskQueue::TaskQueue() noexcept
    : m_is_broken(false) {}

  template<typename T, std::invocable<const T&> F>
  auto TaskQueue::get_slot(F&& callback) {
    return get_slot<T>(
      std::forward<F>(callback), [] (const std::exception_ptr&) {});
  }

  template<typename T>
  auto TaskQueue::get_slot(const std::function<void (const T&)>& callback) {
    return get_slot(callback, std::function<void (const std::exception_ptr&)>(
      [] (const std::exception_ptr&) {}));
  }

  template<typename T, std::invocable<const T&> F,
    std::invocable<const std::exception_ptr&> B>
  auto TaskQueue::get_slot(F&& callback, B&& on_break) {
    return get_slot_helper<T>(
      std::forward<F>(callback), std::forward<B>(on_break));
  }

  template<typename T>
  auto TaskQueue::get_slot(const std::function<void (const T&)>& callback,
      const std::function<void (const std::exception_ptr&)>& on_break) {
    return get_slot_helper<T>(callback, on_break);
  }

  inline TaskQueue::Source TaskQueue::pop() {
    return m_tasks.pop();
  }

  inline boost::optional<TaskQueue::Source> TaskQueue::try_pop() {
    return m_tasks.try_pop();
  }

  inline void TaskQueue::push(const Target& value) {
    m_tasks.push(value);
  }

  inline void TaskQueue::push(Target&& value) {
    m_tasks.push(std::move(value));
  }

  inline void TaskQueue::close(const std::exception_ptr& exception) {
    if(!m_is_broken.exchange(true)) {
      m_callbacks.close(exception);
      push([=, this] {
        m_tasks.close(exception);
      });
    }
  }

  template<typename T, typename F, typename B>
  auto TaskQueue::get_slot_helper(F&& callback, B&& on_break) {
    return m_callbacks.get_slot<T>(
      [this, callback = std::make_shared<std::remove_cvref_t<F>>(
          std::forward<F>(callback))] (const T& value) {
        m_tasks.push([=, this] {
          (*callback)(value);
        });
      },
      [this, on_break = std::make_shared<std::remove_cvref_t<B>>(
          std::forward<B>(on_break))] (const std::exception_ptr& e) {
        m_tasks.push([=, this] () {
          (*on_break)(e);
        });
      });
  }
}

#endif
