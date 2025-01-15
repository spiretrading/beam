#ifndef BEAM_ROUTINE_HANDLER_HPP
#define BEAM_ROUTINE_HANDLER_HPP
#include <atomic>
#include <condition_variable>
#include <mutex>
#include "Beam/Routines/Routines.hpp"
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/Scheduler.hpp"

namespace Beam::Routines {

  /** Used to spawn a Routine and wait for its completion. */
  class RoutineHandler {
    public:

      /** Constructs a RoutineHandler. */
      RoutineHandler();

      /**
       * Constructs a RoutineHandler.
       * @param id The Id of the Routine to manage.
       */
      RoutineHandler(Routine::Id id);

      /**
       * Acquires a RoutineHandler.
       * @param routineHandler The RoutineHandler to acquire.
       */
      RoutineHandler(RoutineHandler&& routineHandler);

      ~RoutineHandler();

      /**
       * Assigns a Routine to this handler.
       * @param id The Id of the Routine to handle.
       */
      RoutineHandler& operator =(Routine::Id id);

      /**
       * Acquires a RoutineHandler.
       * @param routineHandler The RoutineHandler to acquire.
       */
      RoutineHandler& operator =(RoutineHandler&& routineHandler);

      /** Returns the Routine's id. */
      Routine::Id GetId() const;

      /** Detaches the current Routine from this handler. */
      void Detach();

      /** Waits for the completion of the previously spawned Routine. */
      void Wait();

    private:
      Routine::Id m_id;

      RoutineHandler(const RoutineHandler&) = delete;
      RoutineHandler& operator =(const RoutineHandler&) = delete;
  };

  /** Waits for all pending Routines to complete. */
  inline void FlushPendingRoutines() {
    auto& scheduler = Details::Scheduler::GetInstance();
    auto threadCountMutex = std::mutex();
    auto threadCountCondition = std::condition_variable();
    while(true) {
      auto threadCount = scheduler.GetThreadCount();
      auto routines = std::vector<RoutineHandler>();
      auto isComplete = std::atomic_bool(true);
      for(auto i = std::size_t(0); i < scheduler.GetThreadCount(); ++i) {
        routines.emplace_back(Spawn([&] {
          auto& routine = static_cast<ScheduledRoutine&>(GetCurrentRoutine());
          {
            auto lock = std::unique_lock(threadCountMutex);
            --threadCount;
            if(threadCount == 0) {
              threadCountCondition.notify_all();
            } else {
              while(threadCount != 0) {
                threadCountCondition.wait(lock);
              }
            }
          }
          if(scheduler.HasPendingRoutines(routine.GetContextId())) {
            isComplete = false;
          }
        }, Details::Scheduler::DEFAULT_STACK_SIZE, i));
      }
      routines.clear();
      if(isComplete) {
        break;
      }
    }
  }

  inline RoutineHandler::RoutineHandler()
    : RoutineHandler(0) {}

  inline RoutineHandler::RoutineHandler(Routine::Id id)
    : m_id(id) {}

  inline RoutineHandler::RoutineHandler(RoutineHandler&& routineHandler)
      : RoutineHandler(routineHandler.m_id) {
    routineHandler.Detach();
  }

  inline RoutineHandler::~RoutineHandler() {
    Wait();
  }

  inline RoutineHandler& RoutineHandler::operator =(Routine::Id id) {
    if(m_id == id) {
      return *this;
    }
    Wait();
    m_id = id;
    return *this;
  }

  inline RoutineHandler& RoutineHandler::operator =(
      RoutineHandler&& routineHandler) {
    if(this == &routineHandler) {
      return *this;
    }
    *this = routineHandler.m_id;
    routineHandler.Detach();
    return *this;
  }

  inline Routine::Id RoutineHandler::GetId() const {
    return m_id;
  }

  inline void RoutineHandler::Detach() {
    m_id = 0;
  }

  inline void RoutineHandler::Wait() {
    if(m_id == 0) {
      return;
    }
    Details::Scheduler::GetInstance().Wait(m_id);
    m_id = 0;
  }
}

#endif
