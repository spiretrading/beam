#ifndef BEAM_TASKRUNNER_HPP
#define BEAM_TASKRUNNER_HPP
#include <deque>
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Threading/Threading.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {
namespace Threading {

  /*! \class TaskRunner
      \brief Runs a series of tasks that get pushed.
   */
  class TaskRunner : private boost::noncopyable {
    public:

      //! Defines the type of a Task.
      using Task = std::function<void ()>;

      //! Constructs a TaskRunner.
      TaskRunner();

      ~TaskRunner();

      //! Adds a task.
      /*!
        \param task The task to perform.
      */
      template<typename F>
      void Add(F&& task);

      //! Returns <code>true</code> iff no Tasks remain pending or running.
      bool IsEmpty() const;

    private:
      mutable boost::mutex m_mutex;
      bool m_handlingTasks;
      std::deque<Task> m_pendingTasks;
      boost::condition_variable m_handlingTaskCondition;

      void HandleTasks(boost::unique_lock<boost::mutex>& lock);
  };

  inline TaskRunner::TaskRunner()
      : m_handlingTasks(false) {}

  inline TaskRunner::~TaskRunner() {
    boost::unique_lock<boost::mutex> lock(m_mutex);
    while(m_handlingTasks || !m_pendingTasks.empty()) {
      m_handlingTaskCondition.wait(lock);
    }
  }

  template<typename F>
  inline void TaskRunner::Add(F&& task) {
    boost::unique_lock<boost::mutex> lock(m_mutex);
    m_pendingTasks.push_back(std::forward<F>(task));
    if(m_handlingTasks) {
      return;
    }
    HandleTasks(lock);
  }

  inline bool TaskRunner::IsEmpty() const {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    return m_pendingTasks.empty() && !m_handlingTasks;
  }

  inline void TaskRunner::HandleTasks(boost::unique_lock<boost::mutex>& lock) {
    m_handlingTasks = true;
    while(!m_pendingTasks.empty()) {
      Task task(std::move(m_pendingTasks.front()));
      m_pendingTasks.pop_front();
      {
        LockRelease<boost::unique_lock<boost::mutex>> release(lock);
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
}

#endif
