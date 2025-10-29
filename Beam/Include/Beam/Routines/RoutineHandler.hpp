#ifndef BEAM_ROUTINE_HANDLER_HPP
#define BEAM_ROUTINE_HANDLER_HPP
#include <atomic>
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/Scheduler.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {

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

  /** Waits for all pending Routines to complete. */
  inline void flush_pending_routines() {
    auto& scheduler = Details::Scheduler::get();
    auto thread_count_mutex = Mutex();
    auto thread_count_condition = ConditionVariable();
    while(true) {
      auto thread_count = scheduler.get_thread_count();
      auto routines = std::vector<RoutineHandler>();
      auto is_complete = std::atomic_bool(true);
      for(auto i = std::size_t(0); i < scheduler.get_thread_count(); ++i) {
        routines.emplace_back(spawn([&] {
          auto& routine = static_cast<ScheduledRoutine&>(get_current_routine());
          {
            auto lock = boost::unique_lock(thread_count_mutex);
            --thread_count;
            if(thread_count == 0) {
              thread_count_condition.notify_all();
            } else {
              while(thread_count != 0) {
                thread_count_condition.wait(lock);
              }
            }
          }
          if(scheduler.has_pending_routines(routine.get_context_id())) {
            is_complete = false;
          }
        }, Details::Scheduler::DEFAULT_STACK_SIZE, i));
      }
      routines.clear();
      if(is_complete) {
        break;
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
