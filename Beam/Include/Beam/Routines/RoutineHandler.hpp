#ifndef BEAM_ROUTINE_HANDLER_HPP
#define BEAM_ROUTINE_HANDLER_HPP
#include <atomic>
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/Scheduler.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Threading/ThreadPool.hpp"

namespace Beam {
namespace Details {
  struct BEAM_EXPORT_DLL FlushMutex {
    static Mutex& get();
  };

#ifndef BEAM_USE_DLL
  BEAM_EMIT_DLL inline Mutex& FlushMutex::get() {
    static auto mutex = Mutex();
    return mutex;
  }
#endif
}

  /** Used to spawn a Routine and wait for its completion. */
  class RoutineHandler {
    public:

      /** Constructs a RoutineHandler. */
      RoutineHandler() noexcept;

      /**
       * Constructs a RoutineHandler.
       * @param id The Id of the Routine to manage.
       */
      RoutineHandler(Routine::Id id) noexcept;

      RoutineHandler(RoutineHandler&& handler) noexcept;
      ~RoutineHandler();

      /** Returns the Routine's id. */
      Routine::Id get_id() const;

      /** Detaches the current Routine from this handler. */
      void detach();

      /** Waits for the completion of the previously spawned Routine. */
      void wait();

      RoutineHandler& operator =(Routine::Id id) noexcept;
      RoutineHandler& operator =(RoutineHandler&& handler) noexcept;

    private:
      Routine::Id m_id;

      RoutineHandler(const RoutineHandler&) = delete;
      RoutineHandler& operator =(const RoutineHandler&) = delete;
  };

  /**
   * Waits for all pending Routines to complete, including Routines suspended
   * on blocking tasks parked in the ThreadPool.
   */
  inline void flush_pending_routines() {
    auto& scheduler = Details::Scheduler::get();
    auto& thread_pool = ThreadPool::get();
    auto lock = boost::lock_guard(Details::FlushMutex::get());
    auto target = [] {
      if(dynamic_cast<ScheduledRoutine*>(Details::CurrentRoutineGlobal::get())) {
        return 1;
      }
      return 0;
    }();
    while(true) {
      auto queued_count = thread_pool.get_queued_count();
      thread_pool.wait_until_idle();
      if(scheduler.get_busy_count() == target && thread_pool.is_idle() &&
          thread_pool.get_queued_count() == queued_count) {
        break;
      }
      if(scheduler.get_busy_count() != target) {
        auto idle = Async<void>();
        scheduler.notify_when_idle(idle.get_eval());
        idle.get();
      }
    }
  }

  inline RoutineHandler::RoutineHandler() noexcept
    : RoutineHandler(0) {}

  inline RoutineHandler::RoutineHandler(Routine::Id id) noexcept
    : m_id(id) {}

  inline RoutineHandler::RoutineHandler(RoutineHandler&& handler) noexcept
      : RoutineHandler(handler.m_id) {
    handler.detach();
  }

  inline RoutineHandler::~RoutineHandler() {
    wait();
  }

  inline Routine::Id RoutineHandler::get_id() const {
    return m_id;
  }

  inline void RoutineHandler::detach() {
    m_id = 0;
  }

  inline void RoutineHandler::wait() {
    if(m_id == 0) {
      return;
    }
    Details::Scheduler::get().wait(m_id);
    m_id = 0;
  }

  inline RoutineHandler& RoutineHandler::operator =(Routine::Id id) noexcept {
    if(m_id == id) {
      return *this;
    }
    wait();
    m_id = id;
    return *this;
  }

  inline RoutineHandler& RoutineHandler::operator =(
      RoutineHandler&& handler) noexcept {
    if(this == &handler) {
      return *this;
    }
    *this = handler.m_id;
    handler.detach();
    return *this;
  }
}

#endif
