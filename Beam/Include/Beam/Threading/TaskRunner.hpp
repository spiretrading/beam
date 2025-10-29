#ifndef BEAM_TASK_RUNNER_HPP
#define BEAM_TASK_RUNNER_HPP
#include <concepts>
#include <deque>
#include <functional>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {

  /** Runs a series of tasks that get pushed. */
  class TaskRunner {
    public:

      /** Defines the type of a Task. */
      using Task = std::function<void ()>;

      /** Constructs a TaskRunner. */
      TaskRunner() noexcept;

      ~TaskRunner();

      /**
       * Adds a task.
       * @param task The task to perform.
       */
      template<std::invocable<> F>
      void add(F&& task);

      /** Returns <code>true</code> iff no Tasks remain pending or running. */
      bool is_empty() const;

    private:
      mutable boost::mutex m_mutex;
      bool m_handling_tasks;
      std::deque<Task> m_pending_tasks;
      boost::condition_variable m_handling_task_condition;

      TaskRunner(const TaskRunner&) = delete;
      TaskRunner& operator =(const TaskRunner&) = delete;
      void handle(boost::unique_lock<boost::mutex>& lock);
  };

  inline TaskRunner::TaskRunner() noexcept
    : m_handling_tasks(false) {}

  inline TaskRunner::~TaskRunner() {
    auto lock = boost::unique_lock(m_mutex);
    while(m_handling_tasks || !m_pending_tasks.empty()) {
      m_handling_task_condition.wait(lock);
    }
  }

  template<std::invocable<> F>
  void TaskRunner::add(F&& task) {
    auto lock = boost::unique_lock(m_mutex);
    m_pending_tasks.emplace_back(std::forward<F>(task));
    if(m_handling_tasks) {
      return;
    }
    handle(lock);
  }

  inline bool TaskRunner::is_empty() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_pending_tasks.empty() && !m_handling_tasks;
  }

  inline void TaskRunner::handle(boost::unique_lock<boost::mutex>& lock) {
    m_handling_tasks = true;
    while(!m_pending_tasks.empty()) {
      auto task = std::move(m_pending_tasks.front());
      m_pending_tasks.pop_front();
      {
        auto releaser = release(lock);
        try {
          task();
        } catch(const std::exception&) {
          std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        }
      }
    }
    m_handling_tasks = false;
    m_handling_task_condition.notify_all();
  }
}

#endif
