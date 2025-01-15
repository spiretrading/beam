#ifndef BEAM_EXTERNAL_ROUTINE_HPP
#define BEAM_EXTERNAL_ROUTINE_HPP
#include <condition_variable>
#include <mutex>
#include "Beam/Routines/Routine.hpp"

namespace Beam::Routines {

  /** Represents a Routine that is not run from within a Scheduler. */
  class ExternalRoutine final : public Routine {
    public:

      /** Constructs an ExternalRoutine. */
      ExternalRoutine();

      ~ExternalRoutine() override;

      bool IsScheduled() const override;

    protected:
      void Execute() override;

      void Defer() override;

      void PendingSuspend() override;

      void Suspend() override;

      void Resume() override;

    private:
      mutable std::mutex m_mutex;
      mutable std::condition_variable m_suspendedCondition;
      bool m_isPendingResume;
  };

  inline ExternalRoutine::ExternalRoutine()
    : m_isPendingResume(false) {}

  inline ExternalRoutine::~ExternalRoutine() {
    SetState(State::COMPLETE);
  }

  inline bool ExternalRoutine::IsScheduled() const {
    return false;
  }

  inline void ExternalRoutine::Execute() {
    SetState(State::RUNNING);
  }

  inline void ExternalRoutine::Defer() {}

  inline void ExternalRoutine::PendingSuspend() {
    SetState(State::PENDING_SUSPEND);
  }

  inline void ExternalRoutine::Suspend() {
    auto lock = std::unique_lock(m_mutex);
    SetState(State::SUSPENDED);
    if(m_isPendingResume) {
      m_isPendingResume = false;
      return;
    }
    while(GetState() == State::SUSPENDED) {
      m_suspendedCondition.wait(lock);
    }
  }

  inline void ExternalRoutine::Resume() {
    auto lock = std::lock_guard(m_mutex);
    if(GetState() == State::PENDING_SUSPEND) {
      m_isPendingResume = true;
      return;
    }
    assert(GetState() == State::SUSPENDED);
    SetState(State::RUNNING);
    m_suspendedCondition.notify_one();
  }

  inline Routine& GetCurrentRoutine() {
    auto routine = Details::CurrentRoutineGlobal<void>::GetInstance();
    if(!routine) {
      thread_local auto externalRoutine = std::make_unique<ExternalRoutine>();
      routine = externalRoutine.get();
      Details::CurrentRoutineGlobal<void>::GetInstance() = routine;
    }
    return *routine;
  }
}

#endif
