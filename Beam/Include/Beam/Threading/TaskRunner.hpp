#ifndef BEAM_TASK_RUNNER_HPP
#define BEAM_TASK_RUNNER_HPP
#include <deque>
#include <functional>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Threading/Threading.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam::Threading {

  /** Runs a series of tasks that get pushed. */
  class TaskRunner {
    public:

      /** Defines the type of a Task. */
      using Task = std::function<void ()>;

      /** Constructs a TaskRunner. */
      TaskRunner();

      ~TaskRunner();

      /**
       * Adds a task.
       * @param task The task to perform.
       */
      template<typename F>
      void Add(F&& task);

      /** Returns <code>true</code> iff no Tasks remain pending or running. */
      bool IsEmpty() const;

    private:
      mutable boost::mutex m_mutex;
      bool m_handlingTasks;
      std::deque<Task> m_pendingTasks;
      boost::condition_variable m_handlingTaskCondition;

      TaskRunner(const TaskRunner&) = delete;
      TaskRunner& operator =(const TaskRunner&) = delete;
      void HandleTasks(boost::unique_lock<boost::mutex>& lock);
  };

  inline TaskRunner::TaskRunner()
    : m_handlingTasks(false) {}

  inline TaskRunner::~TaskRunner() {
    auto lock = boost::unique_lock(m_mutex);
    while(m_handlingTasks || !m_pendingTasks.empty()) {
      m_handlingTaskCondition.wait(lock);
    }
  }

  template<typename F>
  inline void TaskRunner::Add(F&& task) {
    auto lock = boost::unique_lock(m_mutex);
    m_pendingTasks.push_back(std::forward<F>(task));
    if(m_handlingTasks) {
      return;
    }
    HandleTasks(lock);
  }

  inline bool TaskRunner::IsEmpty() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_pendingTasks.empty() && !m_handlingTasks;
  }

  inline void TaskRunner::HandleTasks(boost::unique_lock<boost::mutex>& lock) {
    m_handlingTasks = true;
    while(!m_pendingTasks.empty()) {
      auto task = std::move(m_pendingTasks.front());
      m_pendingTasks.pop_front();
      {
        auto release = Release(lock);
        try {
          task();
        } catch(const std::exception&) {
          std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        }
      }
    }
    m_handlingTasks = false;
    m_handlingTaskCondition.notify_all();
  }
}

#endif
