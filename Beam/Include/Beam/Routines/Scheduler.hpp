#ifndef BEAM_SCHEDULER_HPP
#define BEAM_SCHEDULER_HPP
#include <cstdint>
#include <deque>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <boost/atomic/atomic.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include "Beam/Pointers/UniquePtr.hpp"
#include "Beam/Routines/FunctionRoutine.hpp"
#include "Beam/Routines/Routines.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Utilities/ReportException.hpp"
#include "Beam/Utilities/Singleton.hpp"

namespace Beam {
namespace Routines {
namespace Details {

  /*! \class Scheduler
      \brief Schedules the execution of Routines across multiple threads.
   */
  class Scheduler : public Singleton<Scheduler> {
    public:

      //! The default size of a Routine's stack.
      static const std::size_t DEFAULT_STACK_SIZE = 64 * 1024;

      //! Constructs a Scheduler with a number of threads equal to the system's
      //! concurrency.
      Scheduler();

      ~Scheduler();

      //! Waits for a Routine to complete.
      /*!
        \param id The id of the Routine to wait for.
      */
      void Wait(Routine::Id id);

      //! Spawns a Routine from a callable object.
      /*!
        \param f The callable object to run within the Routine.
        \return A unique ID used to identify the Routine.
      */
      template<typename F>
      Routine::Id Spawn(F&& f);

      //! Spawns a Routine from a callable object.
      /*!
        \param f The callable object to run within the Routine.
        \param stackSize The size of the stack to allocate for the Routine.
        \return A unique ID used to identify the Routine.
      */
      template<typename F>
      Routine::Id Spawn(F&& f, std::size_t stackSize);

      //! Waits for any currently executing Routines to COMPLETE and stops
      //! executing any new ones.
      void Stop();

    private:
      struct Context {
        std::shared_ptr<boost::mutex> m_mutex;
        bool m_isRunning;
        std::deque<ScheduledRoutine*> m_pendingRoutines;
        std::unordered_set<ScheduledRoutine*> m_suspendedRoutines;
        std::shared_ptr<boost::condition_variable>
          m_pendingRoutinesAvailableCondition;

        Context();
      };
      typedef std::unordered_map<Routine::Id, ScheduledRoutine*> RoutineIds;
      friend class Beam::Routines::ScheduledRoutine;
      friend void Resume(ScheduledRoutine*& routine);
      std::vector<boost::thread> m_threads;
      std::size_t m_threadCount;
      Threading::Sync<RoutineIds> m_routineIds;
      std::vector<Context> m_contexts;

      void Queue(ScheduledRoutine& routine);
      void Suspend(ScheduledRoutine& routine);
      void Resume(ScheduledRoutine& routine);
      void Run(Context& context);
  };

  inline Scheduler::Context::Context()
      : m_mutex{std::make_shared<boost::mutex>()},
        m_isRunning{true},
        m_pendingRoutinesAvailableCondition{
          std::make_shared<boost::condition_variable>()} {}

  inline Scheduler::Scheduler()
      : m_threadCount{boost::thread::hardware_concurrency()},
        m_contexts{m_threadCount} {
    for(std::size_t i = 0; i < m_threadCount; ++i) {
      m_threads.emplace_back(
        [=] {
          Run(m_contexts[i]);
        });
    }
  }

  inline Scheduler::~Scheduler() {
    Stop();
  }

  inline void Scheduler::Wait(Routine::Id id) {
    assert(GetCurrentRoutine().GetId() != id);
    Async<void> waitAsync;
    bool wait;
    Threading::With(m_routineIds,
      [&] (RoutineIds& routineIds) {
        auto routineIterator = routineIds.find(id);
        if(routineIterator == routineIds.end()) {
          wait = false;
          return;
        }
        ScheduledRoutine* routine = routineIterator->second;
        routine->Wait(waitAsync.GetEval());
        wait = true;
      });
    if(wait) {
      waitAsync.Get();
    }
  }

  template<typename F>
  Routine::Id Scheduler::Spawn(F&& f) {
    return Spawn(std::forward<F>(f), DEFAULT_STACK_SIZE);
  }

  template<typename F>
  Routine::Id Scheduler::Spawn(F&& f, std::size_t stackSize) {
    ScheduledRoutine* routine = new FunctionRoutine<F>(std::forward<F>(f),
      m_threadCount, stackSize, Ref(*this));
    auto id = routine->GetId();
    Threading::With(m_routineIds,
      [&] (RoutineIds& routineIds) {
        routineIds.insert(std::make_pair(id, routine));
      });
    Queue(*routine);
    return id;
  }

  inline void Scheduler::Queue(ScheduledRoutine& routine) {
    Context& context = m_contexts[routine.GetContextId()];
    boost::lock_guard<boost::mutex> lock(*context.m_mutex);
    context.m_pendingRoutines.push_back(&routine);
    if(context.m_pendingRoutines.size() == 1) {
      context.m_pendingRoutinesAvailableCondition->notify_all();
    }
  }

  inline void Scheduler::Suspend(ScheduledRoutine& routine) {
    Context& context = m_contexts[routine.GetContextId()];
    boost::lock_guard<boost::mutex> lock(*context.m_mutex);
    routine.SetState(Routine::State::SUSPENDED);
    if(routine.IsPendingResume()) {
      routine.SetPendingResume(false);
      context.m_pendingRoutines.push_back(&routine);
      context.m_pendingRoutinesAvailableCondition->notify_all();
      return;
    }
    context.m_suspendedRoutines.insert(&routine);
  }

  inline void Scheduler::Resume(ScheduledRoutine& routine) {
    Context& context = m_contexts[routine.GetContextId()];
    boost::lock_guard<boost::mutex> lock(*context.m_mutex);
    auto routineIterator = context.m_suspendedRoutines.find(&routine);
    if(routineIterator == context.m_suspendedRoutines.end()) {
      routine.SetPendingResume(true);
      return;
    }
    context.m_suspendedRoutines.erase(routineIterator);
    context.m_pendingRoutines.push_back(&routine);
    context.m_pendingRoutinesAvailableCondition->notify_all();
  }

  inline void Scheduler::Stop() {
    for(Context& context : m_contexts) {
      boost::lock_guard<boost::mutex> contextLock(*context.m_mutex);
      context.m_isRunning = false;
      context.m_pendingRoutinesAvailableCondition->notify_all();
    }
    for(auto& thread : m_threads) {
      thread.join();
    }
    for(Context& context : m_contexts) {
      boost::lock_guard<boost::mutex> contextLock(*context.m_mutex);
    }
  }

  inline void Scheduler::Run(Context& context) {
    while(true) {
      ScheduledRoutine* routine;
      {
        boost::unique_lock<boost::mutex> lock(*context.m_mutex);
        while(context.m_pendingRoutines.empty()) {
          if(!context.m_isRunning && context.m_pendingRoutines.empty() &&
              context.m_suspendedRoutines.empty()) {
            return;
          }
          context.m_pendingRoutinesAvailableCondition->wait(lock);
        }
        routine = context.m_pendingRoutines.front();
        context.m_pendingRoutines.pop_front();
      }
      routine->Continue();
      if(routine->GetState() == Routine::State::COMPLETE) {
        Threading::With(m_routineIds,
          [&] (RoutineIds& routineIds) {
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
  Routine::Id Spawn(F&& f) {
    return Details::Scheduler::GetInstance().Spawn(std::forward<F>(f));
  }

  template<typename F>
  Routine::Id Spawn(F&& f, std::size_t stackSize) {
    return Details::Scheduler::GetInstance().Spawn(std::forward<F>(f),
      stackSize);
  }

  template<typename F>
  Routine::Id Spawn(F&& f, Eval<decltype(f())> result) {
    return Spawn(std::forward<F>(f), Details::Scheduler::DEFAULT_STACK_SIZE,
      std::move(result));
  }

  template<typename F>
  Routine::Id Spawn(F&& f, std::size_t stackSize, Eval<decltype(f())> result) {
    return Spawn(
      [f = std::move(f), result = std::move(result)] {
        try {
          result.SetResult(f());
        } catch(...) {
          result.SetException(std::current_exception());
        }
      }, stackSize);
  }

  inline void Wait(Routine::Id id) {
    Details::Scheduler::GetInstance().Wait(id);
  }

  inline void ScheduledRoutine::Resume() {
    m_scheduler->Resume(*this);
  }
}
}

#endif
