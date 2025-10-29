#ifndef BEAM_THREAD_POOL_HPP
#define BEAM_THREAD_POOL_HPP
#include <concepts>
#include <deque>
#include <iostream>
#include <type_traits>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include "Beam/Routines/Async.hpp"
#include "Beam/Utilities/Algorithm.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/ReportException.hpp"
#include "Beam/Utilities/Singleton.hpp"

namespace Beam {

  /** Implements a thread pool for running Tasks. */
  class ThreadPool : public Singleton<ThreadPool> {
    public:
      ~ThreadPool();

      /**
       * Queues a function to be run within an allocated thread.
       * @param function The function execute.
       * @param result The result of the <i>function</i>.
       */
      template<std::invocable<> F, typename R> requires
        std::convertible_to<std::invoke_result_t<F>, R>
      void queue(F&& function, Eval<R> result);

    private:
      friend class Singleton<ThreadPool>;
      struct TaskThread;
      struct BaseTask {
        virtual ~BaseTask() = default;

        virtual void run() = 0;
      };
      template<typename F, typename R>
      struct Invoker {
        void operator ()(F& function, Eval<R> result) const;
      };
      template<typename F>
      struct Invoker<F, void> {
        void operator ()(F& function, Eval<void> result) const;
      };
      template<typename F, typename R>
      struct Task : BaseTask {
        F m_function;
        Eval<R> m_result;

        template<typename U>
        Task(U&& function, Eval<R> result);

        void run() override;
      };
      struct TaskThread {
        boost::mutex m_mutex;
        ThreadPool* m_thread_pool;
        BaseTask* m_task;
        bool m_available;
        bool m_stopped;
        boost::thread m_thread;
        boost::condition_variable m_task_available_condition;

        TaskThread(ThreadPool& thread_pool);

        bool set_task(BaseTask& task);
        void run();
        void stop();
      };
      static constexpr auto LOWER_BOUND_WAIT_TIME = 30;
      static constexpr auto UPPER_BOUND_WAIT_TIME = 60;
      boost::mutex m_mutex;
      std::size_t m_max_thread_count;
      bool m_is_waiting_for_completion;
      std::size_t m_running_threads;
      std::size_t m_active_threads;
      std::deque<TaskThread*> m_threads;
      std::deque<BaseTask*> m_tasks;
      boost::condition_variable m_threads_finished_condition;

      ThreadPool();
      ThreadPool(const ThreadPool&) = delete;
      ThreadPool& operator =(const ThreadPool&) = delete;
      void queue(BaseTask& task);
      bool add(TaskThread& task_thread);
      void remove(TaskThread& task_thread);
  };

  /**
   * Invokes a blocking function (usually IO) within a thread pool.
   * @param f The blocking function to invoke.
   * @return The result of invoking <i>f</i>.
   */
  template<std::invocable<> F>
  auto park(F&& f) {
    using Result = std::remove_cvref_t<std::invoke_result_t<F>>;
    auto result = Async<Result>();
    ThreadPool::get().queue(std::forward<F>(f), result.get_eval());
    if constexpr(std::same_as<Result, void>) {
      result.get();
      return;
    } else {
      return std::move(result.get());
    }
  }

  template<typename F, typename R>
  void ThreadPool::Invoker<F, R>::operator ()(
      F& function, Eval<R> result) const {
    try {
      result.set(function());
    } catch(const std::exception&) {
      result.set_exception(std::current_exception());
    }
  }

  template<typename F>
  void ThreadPool::Invoker<F, void>::operator ()(
      F& function, Eval<void> result) const {
    try {
      function();
      result.set();
    } catch(const std::exception&) {
      result.set_exception(std::current_exception());
    }
  }

  template<typename F, typename R>
  template<typename U>
  ThreadPool::Task<F, R>::Task(U&& function, Eval<R> result)
    : m_function(std::forward<U>(function)),
      m_result(std::move(result)) {}

  template<typename F, typename R>
  void ThreadPool::Task<F, R>::run() {
    Invoker<F, R>()(m_function, std::move(m_result));
  }

  inline ThreadPool::TaskThread::TaskThread(ThreadPool& thread_pool)
      : m_thread_pool(&thread_pool),
        m_task(nullptr),
        m_available(true),
        m_stopped(false) {
    m_thread = boost::thread([this] {
      run();
    });
  }

  inline bool ThreadPool::TaskThread::set_task(BaseTask& task) {
    auto lock = boost::lock_guard(m_mutex);
    assert(!m_task);
    if(!m_available) {
      return false;
    }
    m_task = &task;
    m_task_available_condition.notify_one();
    return true;
  }

  inline void ThreadPool::TaskThread::run() {
    while(true) {
      {
        auto lock = boost::unique_lock(m_mutex);
        if(!m_task) {
          auto wait_time = boost::posix_time::seconds(LOWER_BOUND_WAIT_TIME) +
            boost::posix_time::seconds(rand() %
              (UPPER_BOUND_WAIT_TIME - LOWER_BOUND_WAIT_TIME));
          if(!m_available ||
              (!m_task_available_condition.timed_wait(lock, wait_time) &&
                !m_task) || !m_available) {
            m_available = false;
            auto stopped = m_stopped;
            lock.unlock();
            if(!stopped) {
              m_thread_pool->remove(*this);
            }
            delete this;
            return;
          }
        }
        assert(m_available);
        try {
          m_task->run();
        } catch(...) {
          std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        }
        delete m_task;
        m_task = nullptr;
      }
      if(!m_thread_pool->add(*this)) {
        delete this;
        return;
      }
    }
  }

  inline void ThreadPool::TaskThread::stop() {
    auto lock = boost::lock_guard(m_mutex);
    m_available = false;
    m_stopped = true;
    m_task = nullptr;
    m_task_available_condition.notify_all();
  }

  inline ThreadPool::~ThreadPool() {
    auto lock = boost::unique_lock(m_mutex);
    m_is_waiting_for_completion = true;
    m_max_thread_count = 0;
    while(!m_threads.empty()) {
      auto task_thread = m_threads.back();
      task_thread->stop();
      m_threads.pop_back();
    }
    if(m_active_threads != 0) {
      m_threads_finished_condition.wait(lock);
    }
  }

  inline ThreadPool::ThreadPool()
    : m_max_thread_count(boost::thread::hardware_concurrency()),
      m_running_threads(0),
      m_active_threads(0),
      m_is_waiting_for_completion(false) {}

  template<std::invocable<> F, typename R> requires
    std::convertible_to<std::invoke_result_t<F>, R>
  void ThreadPool::queue(F&& function, Eval<R> result) {
    auto task = new Task<F, R>(std::forward<F>(function), std::move(result));
    queue(*task);
  }

  inline void ThreadPool::queue(BaseTask& task) {
    auto lock = boost::lock_guard(m_mutex);
    auto is_thread_found = false;
    while(!is_thread_found) {
      if(m_threads.empty() && m_running_threads < m_max_thread_count) {
        auto task_thread = new TaskThread(*this);
        task_thread->set_task(task);
        ++m_active_threads;
        ++m_running_threads;
        return;
      } else if(!m_tasks.empty() || m_threads.empty()) {
        m_tasks.push_back(&task);
        return;
      }
      auto task_thread = m_threads.front();
      m_threads.pop_front();
      is_thread_found = task_thread->set_task(task);
      if(is_thread_found) {
        ++m_active_threads;
      }
    }
  }

  inline bool ThreadPool::add(TaskThread& task_thread) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_max_thread_count == 0) {
      --m_active_threads;
      --m_running_threads;
      if(m_active_threads == 0) {
        m_threads_finished_condition.notify_all();
      }
      return false;
    }
    if(m_tasks.empty()) {
      m_threads.push_back(&task_thread);
      --m_active_threads;
      if(m_is_waiting_for_completion && m_active_threads == 0) {
        m_threads_finished_condition.notify_all();
      }
      return true;
    }
    auto task = m_tasks.front();
    if(!task_thread.set_task(*task)) {
      --m_active_threads;
      --m_running_threads;
      return false;
    }
    m_tasks.pop_front();
    return true;
  }

  inline void ThreadPool::remove(TaskThread& task_thread) {
    auto lock = boost::lock_guard(m_mutex);
    --m_running_threads;
    remove_first(m_threads, &task_thread);
  }
}

#endif
