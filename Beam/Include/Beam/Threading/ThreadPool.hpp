#ifndef BEAM_THREAD_POOL_HPP
#define BEAM_THREAD_POOL_HPP
#include <algorithm>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include "Beam/Routines/Async.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/DllExport.hpp"
#include "Beam/Utilities/ReportException.hpp"
#include "Beam/Utilities/Singleton.hpp"

namespace Beam {
namespace Tests {
  struct ThreadPoolTester;
}

  /** Implements a thread pool for running Tasks. */
  class BEAM_EXPORT_DLL ThreadPool : public Singleton<ThreadPool> {
    public:
      ~ThreadPool();

      /** Returns the total number of tasks ever queued. */
      std::uint64_t get_queued_count() const;

      /** Returns <code>true</code> iff no task is running or waiting to run. */
      bool is_idle() const;

      /**
       * Queues a function to be run within an allocated thread.
       * @param function The function execute.
       * @param result The result of the <i>function</i>.
       */
      template<std::invocable<> F, typename R> requires
        std::convertible_to<std::invoke_result_t<F>, R>
      void queue(F&& function, Eval<R> result);

      /** Blocks until no task is running or waiting to run. */
      void wait_until_idle();

    private:
      friend class Singleton<ThreadPool>;
      friend struct Tests::ThreadPoolTester;
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
        std::mutex m_mutex;
        ThreadPool* m_thread_pool;
        BaseTask* m_task;
        bool m_available;
        std::thread m_thread;
        std::condition_variable m_task_available_condition;

        TaskThread(ThreadPool& thread_pool, BaseTask& task);

        bool set_task(BaseTask& task);
        void run();
        void stop();
      };
      static constexpr auto LOWER_BOUND_WAIT_TIME = 30;
      static constexpr auto UPPER_BOUND_WAIT_TIME = 60;
      mutable std::mutex m_mutex;
      std::size_t m_max_thread_count;
      bool m_is_waiting_for_completion;
      std::size_t m_running_threads;
      std::size_t m_active_threads;
      std::uint64_t m_queued_count;
      std::deque<TaskThread*> m_threads;
      std::deque<BaseTask*> m_tasks;
      std::condition_variable m_threads_finished_condition;

      ThreadPool();
      ThreadPool(const ThreadPool&) = delete;
      ThreadPool& operator =(const ThreadPool&) = delete;
      void queue(std::unique_ptr<BaseTask> task);
      bool add(TaskThread& task_thread);
      bool remove(TaskThread& task_thread);
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

  inline ThreadPool::TaskThread::TaskThread(
      ThreadPool& thread_pool, BaseTask& task)
      : m_thread_pool(&thread_pool),
        m_task(&task),
        m_available(true) {
    m_thread = std::thread([this] {
      run();
    });
  }

  inline bool ThreadPool::TaskThread::set_task(BaseTask& task) {
    auto lock = std::lock_guard(m_mutex);
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
        auto lock = std::unique_lock(m_mutex);
        if(!m_task) {
          auto wait_time = std::chrono::seconds(LOWER_BOUND_WAIT_TIME +
            rand() % (UPPER_BOUND_WAIT_TIME - LOWER_BOUND_WAIT_TIME));
          m_task_available_condition.wait_for(lock, wait_time, [&] {
            return m_task || !m_available;
          });
          if(!m_task || !m_available) {
            lock.unlock();
            if(m_thread_pool->remove(*this)) {
              break;
            }
            continue;
          }
        }
        assert(m_available);
        try {
          m_task->run();
        } catch(...) {
          BEAM_LOG_CURRENT_EXCEPTION();
        }
        delete m_task;
        m_task = nullptr;
      }
      if(!m_thread_pool->add(*this)) {
        m_thread_pool->remove(*this);
        break;
      }
    }
    m_thread.detach();
    delete this;
  }

  inline void ThreadPool::TaskThread::stop() {
    auto lock = std::lock_guard(m_mutex);
    m_available = false;
    m_task = nullptr;
    m_task_available_condition.notify_all();
  }

  inline ThreadPool::~ThreadPool() {
    auto lock = std::unique_lock(m_mutex);
    m_is_waiting_for_completion = true;
    m_max_thread_count = 0;
    while(!m_threads.empty()) {
      auto task_thread = m_threads.back();
      task_thread->stop();
      m_threads.pop_back();
    }
    m_threads_finished_condition.wait(lock, [&] {
      return m_running_threads == 0;
    });
  }

  inline ThreadPool::ThreadPool()
    : m_max_thread_count(std::max(1u, std::thread::hardware_concurrency())),
      m_running_threads(0),
      m_active_threads(0),
      m_queued_count(0),
      m_is_waiting_for_completion(false) {}

  inline std::uint64_t ThreadPool::get_queued_count() const {
    auto lock = std::lock_guard(m_mutex);
    return m_queued_count;
  }

  inline bool ThreadPool::is_idle() const {
    auto lock = std::lock_guard(m_mutex);
    return m_active_threads == 0 && m_tasks.empty();
  }

  template<std::invocable<> F, typename R> requires
    std::convertible_to<std::invoke_result_t<F>, R>
  void ThreadPool::queue(F&& function, Eval<R> result) {
    queue(std::make_unique<Task<F, R>>(
      std::forward<F>(function), std::move(result)));
  }

  inline void ThreadPool::wait_until_idle() {
    auto lock = std::unique_lock(m_mutex);
    while(m_active_threads != 0 || !m_tasks.empty()) {
      m_threads_finished_condition.wait(lock);
    }
  }

  inline void ThreadPool::queue(std::unique_ptr<BaseTask> task) {
    auto lock = std::lock_guard(m_mutex);
    ++m_queued_count;
    auto is_thread_found = false;
    while(!is_thread_found) {
      if(m_threads.empty() && m_running_threads < m_max_thread_count) {
        new TaskThread(*this, *task);
        task.release();
        ++m_active_threads;
        ++m_running_threads;
        return;
      } else if(!m_tasks.empty() || m_threads.empty()) {
        m_tasks.push_back(task.get());
        task.release();
        return;
      }
      auto task_thread = m_threads.front();
      m_threads.pop_front();
      is_thread_found = task_thread->set_task(*task);
      if(is_thread_found) {
        task.release();
        ++m_active_threads;
      }
    }
  }

  inline bool ThreadPool::add(TaskThread& task_thread) {
    auto lock = std::lock_guard(m_mutex);
    if(m_max_thread_count == 0) {
      --m_active_threads;
      if(m_active_threads == 0) {
        m_threads_finished_condition.notify_all();
      }
      return false;
    }
    if(m_tasks.empty()) {
      m_threads.push_back(&task_thread);
      --m_active_threads;
      if(m_active_threads == 0) {
        m_threads_finished_condition.notify_all();
      }
      return true;
    }
    auto task = m_tasks.front();
    if(!task_thread.set_task(*task)) {
      --m_active_threads;
      return false;
    }
    m_tasks.pop_front();
    return true;
  }

  inline bool ThreadPool::remove(TaskThread& task_thread) {
    auto lock = std::lock_guard(m_mutex);
    auto thread_lock = std::lock_guard(task_thread.m_mutex);
    if(task_thread.m_task) {
      return false;
    }
    task_thread.m_available = false;
    --m_running_threads;
    auto i = std::find(m_threads.begin(), m_threads.end(), &task_thread);
    if(i != m_threads.end()) {
      *i = m_threads.back();
      m_threads.pop_back();
    }
    if(m_running_threads == 0) {
      m_threads_finished_condition.notify_all();
    }
    return true;
  }
}

#endif
