#ifndef BEAM_EXTERNAL_ROUTINE_HPP
#define BEAM_EXTERNAL_ROUTINE_HPP
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Routines/Routine.hpp"

namespace Beam {
namespace Routines {

  /*! \class ExternalRoutine
      \brief Represents a Routine that is not run from within a Scheduler.
   */
  class ExternalRoutine final : public Routine {
    public:

      //! Constructs an ExternalRoutine.
      ExternalRoutine();

      ~ExternalRoutine() override;

    protected:
      void Execute() override;

      void Defer() override;

      void PendingSuspend() override;

      void Suspend() override;

      void Resume() override;

    private:
      mutable boost::mutex m_mutex;
      mutable boost::condition_variable m_suspendedCondition;
      bool m_isPendingResume;
  };

  inline ExternalRoutine::ExternalRoutine()
      : m_isPendingResume{false} {}

  inline ExternalRoutine::~ExternalRoutine() {
    SetState(State::COMPLETE);
  }

  inline void ExternalRoutine::Execute() {
    SetState(State::RUNNING);
  }

  inline void ExternalRoutine::Defer() {}

  inline void ExternalRoutine::PendingSuspend() {
    SetState(State::PENDING_SUSPEND);
  }

  inline void ExternalRoutine::Suspend() {
    boost::unique_lock<boost::mutex> lock{m_mutex};
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
    boost::lock_guard<boost::mutex> lock{m_mutex};
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
    if(routine == nullptr) {
      routine = new ExternalRoutine();
      Details::CurrentRoutineGlobal<void>::GetInstance() = routine;
    }
    return *routine;
  }
}
}

#endif
