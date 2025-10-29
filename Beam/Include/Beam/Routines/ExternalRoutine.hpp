#ifndef BEAM_EXTERNAL_ROUTINE_HPP
#define BEAM_EXTERNAL_ROUTINE_HPP
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Routines/Routine.hpp"

namespace Beam {

  /** Represents a Routine that is not run from within a Scheduler. */
  class ExternalRoutine final : public Routine {
    public:

      /** Constructs an ExternalRoutine. */
      ExternalRoutine() noexcept;

      ~ExternalRoutine() override;

    protected:
      void execute() override;
      void defer() override;
      void pending_suspend() override;
      void suspend() override;
      void resume() override;

    private:
      mutable boost::mutex m_mutex;
      mutable boost::condition_variable m_suspended_condition;
      bool m_is_pending_resume;
  };

  inline ExternalRoutine::ExternalRoutine() noexcept
    : m_is_pending_resume(false) {}

  inline ExternalRoutine::~ExternalRoutine() {
    set(State::COMPLETE);
  }

  inline void ExternalRoutine::execute() {
    set(State::RUNNING);
  }

  inline void ExternalRoutine::defer() {}

  inline void ExternalRoutine::pending_suspend() {
    set(State::PENDING_SUSPEND);
  }

  inline void ExternalRoutine::suspend() {
    auto lock = boost::unique_lock(m_mutex);
    set(State::SUSPENDED);
    if(m_is_pending_resume) {
      m_is_pending_resume = false;
      return;
    }
    while(get_state() == State::SUSPENDED) {
      m_suspended_condition.wait(lock);
    }
  }

  inline void ExternalRoutine::resume() {
    auto lock = boost::unique_lock(m_mutex);
    if(get_state() == State::PENDING_SUSPEND) {
      m_is_pending_resume = true;
      return;
    }
    assert(get_state() == State::SUSPENDED);
    set(State::RUNNING);
    m_suspended_condition.notify_one();
  }

  inline Routine& get_current_routine() {
    auto routine = Details::CurrentRoutineGlobal::get();
    if(!routine) {
      thread_local auto external_routine = std::make_unique<ExternalRoutine>();
      routine = external_routine.get();
      Details::CurrentRoutineGlobal::get() = routine;
    }
    return *routine;
  }
}

#endif
