#ifndef BEAM_THREADPOOL_HPP
#define BEAM_THREADPOOL_HPP
#include <deque>
#include <functional>
#include <iostream>
#include <type_traits>
#include <boost/noncopyable.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/type_traits.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Utilities/Algorithm.hpp"
#include "Beam/Utilities/Functional.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {
namespace Threading {

  /*! \class ThreadPool
      \brief Implements a thread pool for running Tasks.
   */
  class ThreadPool : private boost::noncopyable {
    public:

      //! Specifies the return type of a function.
      template<typename F>
      struct ReturnType {
        typedef typename boost::function_traits<
          typename std::remove_pointer<F>::type>::result_type type;
      };

      //! Constructs a ThreadPool.
      ThreadPool();

      //! Constructs a ThreadPool.
      /*!
        \param maxThreadCount The maximum number of threads to allocate.
      */
      ThreadPool(std::size_t maxThreadCount);

      ~ThreadPool();

      //! Returns the maximum thread count.
      std::size_t GetMaxThreadCount() const;

      //! Blocks until all running threads have completed.
      void WaitForCompletion();

      //! Queues a function to be run within an allocated thread.
      /*!
        \param function The function execute.
        \param result The result of the <i>function</i>.
      */
      template<typename F, typename R>
      void Queue(F&& function, Routines::Eval<R> result);

    private:
      class TaskThread;
      class BaseTask : private boost::noncopyable {
        public:
          BaseTask();
          virtual ~BaseTask();
          virtual void Run() = 0;
      };
      template<typename F, typename R>
      struct Invoker {
        void operator ()(F& function, Routines::Eval<R> result) const;
      };
      template<typename F>
      struct Invoker<F, void> {
        void operator ()(F& function, Routines::Eval<void> result) const;
      };
      template<typename F, typename R>
      class Task : public BaseTask {
        public:
          template<typename U>
          Task(U&& function, Routines::Eval<R> result);
          void Run();

        private:
          F m_function;
          Routines::Eval<R> m_result;
      };
      class TaskThread {
        public:
          TaskThread(ThreadPool& threadPool);
          bool SetTask(BaseTask& task);
          void Run();
          void Stop();

        private:
          boost::mutex m_mutex;
          ThreadPool* m_threadPool;
          BaseTask* volatile m_task;
          volatile bool m_available;
          volatile bool m_stopped;
          boost::thread m_thread;
          boost::condition_variable m_taskAvailableCondition;
      };
      static const int LOWER_BOUND_WAIT_TIME = 30;
      static const int UPPER_BOUND_WAIT_TIME = 60;
      boost::mutex m_mutex;
      std::size_t m_maxThreadCount;
      std::size_t m_isWaitingForCompletion;
      std::size_t m_runningThreads;
      std::size_t m_activeThreads;
      std::deque<TaskThread*> m_threads;
      std::deque<BaseTask*> m_tasks;
      boost::condition_variable m_threadsFinishedCondition;

      void QueueTask(BaseTask& task);
      bool AddThread(TaskThread& taskThread);
      void RemoveThread(TaskThread& taskThread);
  };

  inline ThreadPool::BaseTask::BaseTask() {}

  inline ThreadPool::BaseTask::~BaseTask() {}

  template<typename F, typename R>
  void ThreadPool::Invoker<F, R>::operator ()(F& function,
      Routines::Eval<R> result) const {
    try {
      result.SetResult(function());
    } catch(const std::exception&) {
      result.SetException(std::current_exception());
    }
  }

  template<typename F>
  void ThreadPool::Invoker<F, void>::operator ()(F& function,
      Routines::Eval<void> result) const {
    try {
      function();
      result.SetResult();
    } catch(const std::exception&) {
      result.SetException(std::current_exception());
    }
  }

  template<typename F, typename R>
  template<typename U>
  ThreadPool::Task<F, R>::Task(U&& function, Routines::Eval<R> result)
      : m_function(std::forward<U>(function)),
        m_result(std::move(result)) {}

  template<typename F, typename R>
  void ThreadPool::Task<F, R>::Run() {
    Invoker<F, R>()(m_function, std::move(m_result));
  }

  inline ThreadPool::TaskThread::TaskThread(ThreadPool& threadPool)
      : m_threadPool(&threadPool),
        m_task(nullptr),
        m_available(true),
        m_stopped(false) {
    m_thread = boost::thread(std::bind(&TaskThread::Run, this));
  }

  inline bool ThreadPool::TaskThread::SetTask(BaseTask& task) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    assert(m_task == nullptr);
    if(!m_available) {
      return false;
    }
    m_task = &task;
    m_taskAvailableCondition.notify_one();
    return true;
  }

  inline void ThreadPool::TaskThread::Run() {
    while(true) {
      {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        if(m_task == nullptr) {
          boost::posix_time::time_duration waitTime =
            boost::posix_time::seconds(LOWER_BOUND_WAIT_TIME) +
            boost::posix_time::seconds(rand() % (UPPER_BOUND_WAIT_TIME -
            LOWER_BOUND_WAIT_TIME));
          if(!m_available ||
              (!m_taskAvailableCondition.timed_wait(lock, waitTime) &&
              m_task == nullptr) || !m_available) {
            m_available = false;
            bool stopped = m_stopped;
            lock.unlock();
            if(!stopped) {
              m_threadPool->RemoveThread(*this);
            }
            delete this;
            return;
          }
        }
        assert(m_available);
        try {
          m_task->Run();
        } catch(...) {
          std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        }
        delete m_task;
        m_task = nullptr;
      }
      if(!m_threadPool->AddThread(*this)) {
        delete this;
        return;
      }
    }
  }

  inline void ThreadPool::TaskThread::Stop() {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    m_available = false;
    m_stopped = true;
    m_task = nullptr;
    m_taskAvailableCondition.notify_all();
  }

  inline ThreadPool::ThreadPool()
      : m_maxThreadCount(boost::thread::hardware_concurrency()),
        m_runningThreads(0),
        m_activeThreads(0),
        m_isWaitingForCompletion(false) {}

  inline ThreadPool::ThreadPool(std::size_t maxThreadCount)
      : m_maxThreadCount(maxThreadCount),
        m_runningThreads(0),
        m_activeThreads(0),
        m_isWaitingForCompletion(false) {}

  inline ThreadPool::~ThreadPool() {
    boost::unique_lock<boost::mutex> lock(m_mutex);
    m_isWaitingForCompletion = true;
    m_maxThreadCount = 0;
    while(!m_threads.empty()) {
      TaskThread* taskThread = m_threads.back();
      taskThread->Stop();
      m_threads.pop_back();
    }
    if(m_activeThreads != 0) {
      m_threadsFinishedCondition.wait(lock);
    }
  }

  inline std::size_t ThreadPool::GetMaxThreadCount() const {
    return m_maxThreadCount;
  }

  inline void ThreadPool::WaitForCompletion() {
    boost::unique_lock<boost::mutex> lock(m_mutex);
    m_isWaitingForCompletion = true;
    while(m_activeThreads != 0) {
      m_threadsFinishedCondition.wait(lock);
    }
    m_isWaitingForCompletion = false;
  }

  template<typename F, typename R>
  void ThreadPool::Queue(F&& function, Routines::Eval<R> result) {
    Task<F, R>* task = new Task<F, R>(std::forward<F>(function),
      std::move(result));
    QueueTask(*task);
  }

  inline void ThreadPool::QueueTask(BaseTask& task) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    bool threadFound = false;
    while(!threadFound) {
      if(m_threads.empty() && m_runningThreads < m_maxThreadCount) {
        TaskThread* taskThread = new TaskThread(*this);
        taskThread->SetTask(task);
        ++m_activeThreads;
        ++m_runningThreads;
        return;
      } else if(!m_tasks.empty() || m_threads.empty()) {
        m_tasks.push_back(&task);
        return;
      }
      TaskThread* taskThread = m_threads.front();
      m_threads.pop_front();
      threadFound = taskThread->SetTask(task);
      if(threadFound) {
        ++m_activeThreads;
      }
    }
  }

  inline bool ThreadPool::AddThread(TaskThread& taskThread) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    if(m_maxThreadCount == 0) {
      --m_activeThreads;
      --m_runningThreads;
      if(m_activeThreads == 0) {
        m_threadsFinishedCondition.notify_all();
      }
      return false;
    }
    if(m_tasks.empty()) {
      m_threads.push_back(&taskThread);
      --m_activeThreads;
      if(m_isWaitingForCompletion && m_activeThreads == 0) {
        m_threadsFinishedCondition.notify_all();
      }
      return true;
    }
    BaseTask* task = m_tasks.front();
    if(!taskThread.SetTask(*task)) {
      --m_activeThreads;
      --m_runningThreads;
      return false;
    }
    m_tasks.pop_front();
    return true;
  }

  inline void ThreadPool::RemoveThread(TaskThread& taskThread) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    --m_runningThreads;
    RemoveFirst(m_threads, &taskThread);
  }
}
}

#endif
