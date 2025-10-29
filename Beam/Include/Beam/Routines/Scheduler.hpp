#ifndef BEAM_SCHEDULER_HPP
#define BEAM_SCHEDULER_HPP
#include <deque>
#include <iostream>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include "Beam/Routines/FunctionRoutine.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Utilities/ReportException.hpp"
#include "Beam/Utilities/Singleton.hpp"

#ifndef BEAM_SCHEDULER_DEFAULT_STACK_SIZE
  #ifdef _WIN32
    #define BEAM_SCHEDULER_DEFAULT_STACK_SIZE 131072
  #else
    #define BEAM_SCHEDULER_DEFAULT_STACK_SIZE 1048576
  #endif
#endif

namespace Beam {
namespace Details {

  /** Schedules the execution of Routines across multiple threads. */
  class Scheduler : public Singleton<Scheduler> {
    public:

      /** The default size of a Routine's stack. */
      static constexpr auto DEFAULT_STACK_SIZE =
        std::size_t(BEAM_SCHEDULER_DEFAULT_STACK_SIZE);

      /**
       * Constructs a Scheduler with a number of threads equal to the system's
       * concurrency.
       */
      Scheduler();

      ~Scheduler();

      /** Returns the number of threads used by the Scheduler. */
      std::size_t get_thread_count() const;

      /**
       * Returns <code>true</code> iff the context with the specified <i>id</i>
       * has Routines pending.
       */
      bool has_pending_routines(std::size_t context_id) const;

      /**
       * Waits for a Routine to complete.
       * @param id The id of the Routine to wait for.
       */
      void wait(Routine::Id id);

      /**
       * Spawns a Routine from a callable object.
       * @param f The callable object to run within the Routine.
       * @param stack_size The size of the stack to allocate for the Routine.
       * @param context_id The specific context id to run the Routine in, or
       *        set to the number of threads to assign it an arbitrary context.
       * @return A unique ID used to identify the Routine.
       */
      template<typename F>
      Routine::Id spawn(F&& f, std::size_t stack_size, std::size_t context_id);

      /**
       * Waits for any currently executing Routines to COMPLETE and stops
       * executing any new ones.
       */
      void stop();

    private:
      struct Context {
        boost::mutex m_mutex;
        bool m_is_running;
        std::deque<ScheduledRoutine*> m_pending_routines;
        std::unordered_set<ScheduledRoutine*> m_suspended_routines;
        boost::condition_variable m_pending_routines_available_condition;

        Context();
      };
      using RoutineIds = std::unordered_map<Routine::Id, ScheduledRoutine*>;
      friend class Beam::ScheduledRoutine;
      friend void resume(ScheduledRoutine*& routine);
      std::size_t m_thread_count;
      std::unique_ptr<boost::thread[]> m_threads;
      Sync<RoutineIds> m_routine_ids;
      std::unique_ptr<Context[]> m_contexts;

      void queue(ScheduledRoutine& routine);
      void suspend(ScheduledRoutine& routine);
      void resume(ScheduledRoutine& routine);
      void run(Context& context);
  };

  inline Scheduler::Context::Context()
    : m_is_running(true) {}

  inline Scheduler::Scheduler()
      : m_thread_count(boost::thread::hardware_concurrency()),
        m_threads(std::make_unique<boost::thread[]>(m_thread_count)),
        m_contexts(std::make_unique<Context[]>(m_thread_count)) {
    for(auto i = std::size_t(0); i < m_thread_count; ++i) {
      m_threads[i] = boost::thread([=, this] {
        run(m_contexts[i]);
      });
    }
  }

  inline Scheduler::~Scheduler() {
    stop();
  }

  inline std::size_t Scheduler::get_thread_count() const {
    return m_thread_count;
  }

  inline bool Scheduler::has_pending_routines(std::size_t context_id) const {
    auto& context = m_contexts[context_id];
    auto lock = boost::lock_guard(context.m_mutex);
    return !context.m_pending_routines.empty();
  }

  inline void Scheduler::wait(Routine::Id id) {
    assert(get_current_routine().get_id() != id);
    auto wait_async = Async<void>();
    auto wait = with(m_routine_ids, [&] (auto& ids) {
      auto i = ids.find(id);
      if(i == ids.end()) {
        return false;
      }
      auto routine = i->second;
      routine->wait(wait_async.get_eval());
      return true;
    });
    if(wait) {
      wait_async.get();
    }
  }

  template<typename F>
  Routine::Id Scheduler::spawn(
      F&& f, std::size_t stack_size, std::size_t context_id) {
    auto routine =
      new FunctionRoutine(std::forward<F>(f), stack_size, context_id);
    auto id = routine->get_id();
    with(m_routine_ids, [&] (auto& ids) {
      ids.insert(std::pair(id, routine));
    });
    queue(*routine);
    return id;
  }

  inline void Scheduler::queue(ScheduledRoutine& routine) {
    auto& context = m_contexts[routine.get_context_id()];
    auto lock = boost::lock_guard(context.m_mutex);
    context.m_pending_routines.push_back(&routine);
    if(context.m_pending_routines.size() == 1) {
      context.m_pending_routines_available_condition.notify_all();
    }
  }

  inline void Scheduler::suspend(ScheduledRoutine& routine) {
    auto& context = m_contexts[routine.get_context_id()];
    auto lock = boost::lock_guard(context.m_mutex);
    routine.set(Routine::State::SUSPENDED);
    if(routine.is_pending_resume()) {
      routine.set_pending_resume(false);
      context.m_pending_routines.push_back(&routine);
      context.m_pending_routines_available_condition.notify_all();
      return;
    }
    context.m_suspended_routines.insert(&routine);
  }

  inline void Scheduler::resume(ScheduledRoutine& routine) {
    auto& context = m_contexts[routine.get_context_id()];
    auto lock = boost::lock_guard(context.m_mutex);
    auto i = context.m_suspended_routines.find(&routine);
    if(i == context.m_suspended_routines.end()) {
      routine.set_pending_resume(true);
      return;
    }
    context.m_suspended_routines.erase(i);
    context.m_pending_routines.push_back(&routine);
    context.m_pending_routines_available_condition.notify_all();
  }

  inline void Scheduler::stop() {
    for(auto i = std::size_t(0); i != m_thread_count; ++i) {
      auto& context = m_contexts[i];
      auto lock = boost::lock_guard(context.m_mutex);
      context.m_is_running = false;
      context.m_pending_routines_available_condition.notify_all();
    }
    for(auto i = std::size_t(0); i != m_thread_count; ++i) {
      m_threads[i].join();
    }
    for(auto i = std::size_t(0); i != m_thread_count; ++i) {
      auto& context = m_contexts[i];
      auto lock = boost::lock_guard(context.m_mutex);
    }
  }

  inline void Scheduler::run(Context& context) {
    while(true) {
      auto routine = static_cast<ScheduledRoutine*>(nullptr);
      {
        auto lock = boost::unique_lock(context.m_mutex);
        while(context.m_pending_routines.empty()) {
          if(!context.m_is_running && context.m_pending_routines.empty() &&
              context.m_suspended_routines.empty()) {
            return;
          }
          context.m_pending_routines_available_condition.wait(lock);
        }
        routine = context.m_pending_routines.front();
        context.m_pending_routines.pop_front();
      }
      routine->advance();
      if(routine->get_state() == Routine::State::COMPLETE) {
        with(m_routine_ids, [&] (auto& ids) {
          ids.erase(routine->get_id());
        });
        delete routine;
      } else if(routine->get_state() == Routine::State::PENDING_SUSPEND) {
        suspend(*routine);
      } else {
        queue(*routine);
      }
    }
  }
}

  template<typename F>
  Routine::Id spawn(F&& f, std::size_t stack_size, std::size_t context_id) {
    return Details::Scheduler::get().spawn(
      std::forward<F>(f), stack_size, context_id);
  }

  template<typename F>
  Routine::Id spawn(F&& f, std::size_t stack_size) {
    return spawn(std::forward<F>(f), stack_size, static_cast<std::size_t>(-1));
  }

  template<typename F>
  Routine::Id spawn(F&& f) {
    return spawn(std::forward<F>(f), Details::Scheduler::DEFAULT_STACK_SIZE);
  }

  template<typename F>
  Routine::Id spawn(F&& f, std::size_t stack_size, std::size_t context_id,
      Eval<std::remove_cvref_t<decltype(f())>> result) {
    return spawn(
      [f = std::forward<F>(f), result = std::move(result)] () mutable {
        using Result = std::remove_cvref_t<decltype(f())>;
        try {
          if constexpr(std::is_same_v<Result, void>) {
            f();
            result.set();
          } else {
            result.set(f());
          }
        } catch(...) {
          result.set_exception(std::current_exception());
        }
      }, stack_size, context_id);
  }

  template<typename F>
  Routine::Id spawn(F&& f, std::size_t stack_size,
      Eval<std::remove_cvref_t<decltype(f())>> result) {
    return spawn(std::forward<F>(f), stack_size, static_cast<std::size_t>(-1),
      std::move(result));
  }

  template<typename F>
  Routine::Id spawn(F&& f, Eval<std::remove_cvref_t<decltype(f())>> result) {
    return spawn(std::forward<F>(f), Details::Scheduler::DEFAULT_STACK_SIZE,
      std::move(result));
  }

  inline void wait(Routine::Id id) {
    Details::Scheduler::get().wait(id);
  }

  inline void ScheduledRoutine::resume() {
    Details::Scheduler::get().resume(*this);
  }
}

#endif
