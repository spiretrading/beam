#ifndef BEAM_THREAD_POOL_HPP
#define BEAM_THREAD_POOL_HPP
#include <deque>
#include <iostream>
#include <type_traits>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include "Beam/Routines/Async.hpp"
#include "Beam/Threading/Threading.hpp"
#include "Beam/Utilities/Algorithm.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/ReportException.hpp"
#include "Beam/Utilities/Singleton.hpp"

namespace Beam::Threading {

  /** Implements a thread pool for running Tasks. */
  class ThreadPool : public Singleton<ThreadPool> {
    public:
      ~ThreadPool();

      /**
       * Queues a function to be run within an allocated thread.
       * @param function The function execute.
       * @param result The result of the <i>function</i>.
       */
      template<typename F, typename R>
      void Queue(F&& function, Routines::Eval<R> result);

    private:
      friend class Singleton<ThreadPool>;
      struct TaskThread;
      struct BaseTask {
        virtual ~BaseTask() = default;
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
      struct Task : BaseTask {
        F m_function;
        Routines::Eval<R> m_result;

        template<typename U>
        Task(U&& function, Routines::Eval<R> result);
        void Run() override;
      };
      struct TaskThread {
        boost::mutex m_mutex;
        ThreadPool* m_threadPool;
        BaseTask* m_task;
        bool m_available;
        bool m_stopped;
        boost::thread m_thread;
        boost::condition_variable m_taskAvailableCondition;

        TaskThread(ThreadPool& threadPool);
        bool SetTask(BaseTask& task);
        void Run();
        void Stop();
      };
      static constexpr auto LOWER_BOUND_WAIT_TIME = 30;
      static constexpr auto UPPER_BOUND_WAIT_TIME = 60;
      boost::mutex m_mutex;
      std::size_t m_maxThreadCount;
      bool m_isWaitingForCompletion;
      std::size_t m_runningThreads;
      std::size_t m_activeThreads;
      std::deque<TaskThread*> m_threads;
      std::deque<BaseTask*> m_tasks;
      boost::condition_variable m_threadsFinishedCondition;

      ThreadPool();
      ThreadPool(const ThreadPool&) = delete;
      ThreadPool& operator =(const ThreadPool&) = delete;
      void QueueTask(BaseTask& task);
      bool AddThread(TaskThread& taskThread);
      void RemoveThread(TaskThread& taskThread);
  };

  /**
   * Invokes a blocking function (usually IO) within a thread pool.
   * @param f The blocking function to invoke.
   * @return The result of invoking <i>f</i>.
   */
  template<typename F>
  auto Park(F&& f) {
    using Result = std::remove_reference_t<std::invoke_result_t<F>>;
    auto result = Routines::Async<Result>();
    ThreadPool::GetInstance().Queue(std::forward<F>(f), result.GetEval());
    return std::move(result.Get());
  }

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
    m_thread = boost::thread([=] {
      Run();
    });
  }

  inline bool ThreadPool::TaskThread::SetTask(BaseTask& task) {
    auto lock = boost::lock_guard(m_mutex);
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
        auto lock = boost::unique_lock(m_mutex);
        if(m_task == nullptr) {
          auto waitTime = boost::posix_time::seconds(LOWER_BOUND_WAIT_TIME) +
            boost::posix_time::seconds(rand() % (UPPER_BOUND_WAIT_TIME -
            LOWER_BOUND_WAIT_TIME));
          if(!m_available ||
              (!m_taskAvailableCondition.timed_wait(lock, waitTime) &&
              m_task == nullptr) || !m_available) {
            m_available = false;
            auto stopped = m_stopped;
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
    auto lock = boost::lock_guard(m_mutex);
    m_available = false;
    m_stopped = true;
    m_task = nullptr;
    m_taskAvailableCondition.notify_all();
  }

  inline ThreadPool::~ThreadPool() {
    auto lock = boost::unique_lock(m_mutex);
    m_isWaitingForCompletion = true;
    m_maxThreadCount = 0;
    while(!m_threads.empty()) {
      auto taskThread = m_threads.back();
      taskThread->Stop();
      m_threads.pop_back();
    }
    if(m_activeThreads != 0) {
      m_threadsFinishedCondition.wait(lock);
    }
  }

  inline ThreadPool::ThreadPool()
    : m_maxThreadCount(boost::thread::hardware_concurrency()),
      m_runningThreads(0),
      m_activeThreads(0),
      m_isWaitingForCompletion(false) {}

  template<typename F, typename R>
  void ThreadPool::Queue(F&& function, Routines::Eval<R> result) {
    auto task = new Task<F, R>(std::forward<F>(function), std::move(result));
    QueueTask(*task);
  }

  inline void ThreadPool::QueueTask(BaseTask& task) {
    auto lock = boost::lock_guard(m_mutex);
    auto threadFound = false;
    while(!threadFound) {
      if(m_threads.empty() && m_runningThreads < m_maxThreadCount) {
        auto taskThread = new TaskThread(*this);
        taskThread->SetTask(task);
        ++m_activeThreads;
        ++m_runningThreads;
        return;
      } else if(!m_tasks.empty() || m_threads.empty()) {
        m_tasks.push_back(&task);
        return;
      }
      auto taskThread = m_threads.front();
      m_threads.pop_front();
      threadFound = taskThread->SetTask(task);
      if(threadFound) {
        ++m_activeThreads;
      }
    }
  }

  inline bool ThreadPool::AddThread(TaskThread& taskThread) {
    auto lock = boost::lock_guard(m_mutex);
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
    auto task = m_tasks.front();
    if(!taskThread.SetTask(*task)) {
      --m_activeThreads;
      --m_runningThreads;
      return false;
    }
    m_tasks.pop_front();
    return true;
  }

  inline void ThreadPool::RemoveThread(TaskThread& taskThread) {
    auto lock = boost::lock_guard(m_mutex);
    --m_runningThreads;
    RemoveFirst(m_threads, &taskThread);
  }
}

#endif
