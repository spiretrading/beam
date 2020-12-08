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
#include "Beam/Routines/Routines.hpp"
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

namespace Beam::Routines {
namespace Details {

  /** Schedules the execution of Routines across multiple threads. */
  class Scheduler : public Singleton<Scheduler> {
    public:

      /** The default size of a Routine's stack. */
      static constexpr std::size_t DEFAULT_STACK_SIZE =
        BEAM_SCHEDULER_DEFAULT_STACK_SIZE;

      /**
       * Constructs a Scheduler with a number of threads equal to the system's
       * concurrency.
       */
      Scheduler();

      ~Scheduler();

      /** Returns the number of threads used by the Scheduler. */
      std::size_t GetThreadCount() const;

      /**
       * Returns <code>true</code> iff the context with the specified <i>id</i>
       * has Routines pending.
       */
      bool HasPendingRoutines(std::size_t contextId) const;

      /**
       * Waits for a Routine to complete.
       * @param id The id of the Routine to wait for.
       */
      void Wait(Routine::Id id);

      /**
       * Spawns a Routine from a callable object.
       * @param f The callable object to run within the Routine.
       * @param stackSize The size of the stack to allocate for the Routine.
       * @param contextId The specific context id to run the Routine in, or
       *        set to the number of threads to assign it an arbitrary context.
       * @return A unique ID used to identify the Routine.
       */
      template<typename F>
      Routine::Id Spawn(F&& f, std::size_t stackSize, std::size_t contextId);

      /**
       * Waits for any currently executing Routines to COMPLETE and stops
       * executing any new ones.
       */
      void Stop();

    private:
      struct Context {
        boost::mutex m_mutex;
        bool m_isRunning;
        std::deque<ScheduledRoutine*> m_pendingRoutines;
        std::unordered_set<ScheduledRoutine*> m_suspendedRoutines;
        boost::condition_variable m_pendingRoutinesAvailableCondition;

        Context();
      };
      using RoutineIds = std::unordered_map<Routine::Id, ScheduledRoutine*>;
      friend class Beam::Routines::ScheduledRoutine;
      friend void Resume(ScheduledRoutine*& routine);
      std::size_t m_threadCount;
      std::unique_ptr<boost::thread[]> m_threads;
      Threading::Sync<RoutineIds> m_routineIds;
      std::unique_ptr<Context[]> m_contexts;

      void Queue(ScheduledRoutine& routine);
      void Suspend(ScheduledRoutine& routine);
      void Resume(ScheduledRoutine& routine);
      void Run(Context& context);
  };

  inline Scheduler::Context::Context()
    : m_isRunning(true) {}

  inline Scheduler::Scheduler()
      : m_threadCount(boost::thread::hardware_concurrency()),
        m_threads(std::make_unique<boost::thread[]>(m_threadCount)),
        m_contexts(std::make_unique<Context[]>(m_threadCount)) {
    for(auto i = std::size_t(0); i < m_threadCount; ++i) {
      m_threads[i] = boost::thread([=] {
        Run(m_contexts[i]);
      });
    }
  }

  inline Scheduler::~Scheduler() {
    Stop();
  }

  inline std::size_t Scheduler::GetThreadCount() const {
    return m_threadCount;
  }

  inline bool Scheduler::HasPendingRoutines(std::size_t contextId) const {
    auto& context = m_contexts[contextId];
    auto lock = boost::lock_guard(context.m_mutex);
    return !context.m_pendingRoutines.empty();
  }

  inline void Scheduler::Wait(Routine::Id id) {
    assert(GetCurrentRoutine().GetId() != id);
    auto waitAsync = Async<void>();
    auto wait = Threading::With(m_routineIds, [&] (auto& routineIds) {
      auto routineIterator = routineIds.find(id);
      if(routineIterator == routineIds.end()) {
        return false;
      }
      auto routine = routineIterator->second;
      routine->Wait(waitAsync.GetEval());
      return true;
    });
    if(wait) {
      waitAsync.Get();
    }
  }

  template<typename F>
  Routine::Id Scheduler::Spawn(F&& f, std::size_t stackSize,
      std::size_t contextId) {
    auto routine = new FunctionRoutine(std::forward<F>(f), stackSize,
      contextId);
    auto id = routine->GetId();
    Threading::With(m_routineIds, [&] (auto& routineIds) {
      routineIds.insert(std::pair(id, routine));
    });
    Queue(*routine);
    return id;
  }

  inline void Scheduler::Queue(ScheduledRoutine& routine) {
    auto& context = m_contexts[routine.GetContextId()];
    auto lock = boost::lock_guard(context.m_mutex);
    context.m_pendingRoutines.push_back(&routine);
    if(context.m_pendingRoutines.size() == 1) {
      context.m_pendingRoutinesAvailableCondition.notify_all();
    }
  }

  inline void Scheduler::Suspend(ScheduledRoutine& routine) {
    auto& context = m_contexts[routine.GetContextId()];
    auto lock = boost::lock_guard(context.m_mutex);
    routine.SetState(Routine::State::SUSPENDED);
    if(routine.IsPendingResume()) {
      routine.SetPendingResume(false);
      context.m_pendingRoutines.push_back(&routine);
      context.m_pendingRoutinesAvailableCondition.notify_all();
      return;
    }
    context.m_suspendedRoutines.insert(&routine);
  }

  inline void Scheduler::Resume(ScheduledRoutine& routine) {
    auto& context = m_contexts[routine.GetContextId()];
    auto lock = boost::lock_guard(context.m_mutex);
    auto routineIterator = context.m_suspendedRoutines.find(&routine);
    if(routineIterator == context.m_suspendedRoutines.end()) {
      routine.SetPendingResume(true);
      return;
    }
    context.m_suspendedRoutines.erase(routineIterator);
    context.m_pendingRoutines.push_back(&routine);
    context.m_pendingRoutinesAvailableCondition.notify_all();
  }

  inline void Scheduler::Stop() {
    for(std::size_t i = 0; i != m_threadCount; ++i) {
      auto& context = m_contexts[i];
      auto lock = boost::lock_guard(context.m_mutex);
      context.m_isRunning = false;
      context.m_pendingRoutinesAvailableCondition.notify_all();
    }
    for(auto i = std::size_t(0); i != m_threadCount; ++i) {
      m_threads[i].join();
    }
    for(auto i = std::size_t(0); i != m_threadCount; ++i) {
      auto& context = m_contexts[i];
      auto lock = boost::lock_guard(context.m_mutex);
    }
  }

  inline void Scheduler::Run(Context& context) {
    while(true) {
      auto routine = static_cast<ScheduledRoutine*>(nullptr);
      {
        auto lock = boost::unique_lock(context.m_mutex);
        while(context.m_pendingRoutines.empty()) {
          if(!context.m_isRunning && context.m_pendingRoutines.empty() &&
              context.m_suspendedRoutines.empty()) {
            return;
          }
          context.m_pendingRoutinesAvailableCondition.wait(lock);
        }
        routine = context.m_pendingRoutines.front();
        context.m_pendingRoutines.pop_front();
      }
      routine->Continue();
      if(routine->GetState() == Routine::State::COMPLETE) {
        Threading::With(m_routineIds, [&] (auto& routineIds) {
          routineIds.erase(routine->GetId());
        });
        delete routine;
      } else if(routine->GetState() == Routine::State::PENDING_SUSPEND) {
        Suspend(*routine);
      } else {
        Queue(*routine);
      }
    }
  }
}

  template<typename F>
  Routine::Id Spawn(F&& f, std::size_t stackSize, std::size_t contextId) {
    return Details::Scheduler::GetInstance().Spawn(std::forward<F>(f),
      stackSize, contextId);
  }

  template<typename F>
  Routine::Id Spawn(F&& f, std::size_t stackSize) {
    return Spawn(std::forward<F>(f), stackSize, static_cast<std::size_t>(-1));
  }

  template<typename F>
  Routine::Id Spawn(F&& f) {
    return Spawn(std::forward<F>(f), Details::Scheduler::DEFAULT_STACK_SIZE);
  }

  template<typename F>
  Routine::Id Spawn(F&& f, std::size_t stackSize, std::size_t contextId,
      Eval<std::remove_reference_t<decltype(f())>> result) {
    return Spawn(
      [f = std::forward<F>(f), result = std::move(result)] () mutable {
        using Result = std::remove_reference_t<decltype(f())>;
        try {
          if constexpr(std::is_same_v<Result, void>) {
            f();
            result.SetResult();
          } else {
            result.SetResult(f());
          }
        } catch(...) {
          result.SetException(std::current_exception());
        }
      }, stackSize, contextId);
  }

  template<typename F>
  Routine::Id Spawn(F&& f, std::size_t stackSize,
      Eval<std::remove_reference_t<decltype(f())>> result) {
    return Spawn(std::forward<F>(f), stackSize, static_cast<std::size_t>(-1),
      std::move(result));
  }

  template<typename F>
  Routine::Id Spawn(F&& f,
      Eval<std::remove_reference_t<decltype(f())>> result) {
    return Spawn(std::forward<F>(f), Details::Scheduler::DEFAULT_STACK_SIZE,
      std::move(result));
  }

  inline void Wait(Routine::Id id) {
    Details::Scheduler::GetInstance().Wait(id);
  }

  inline void ScheduledRoutine::Resume() {
    Details::Scheduler::GetInstance().Resume(*this);
  }
}

#endif
