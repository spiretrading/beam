#ifndef BEAM_SCHEDULED_ROUTINE_HPP
#define BEAM_SCHEDULED_ROUTINE_HPP
#include <iostream>
#if defined _MSC_VER
#define BEAM_DISABLE_OPTIMIZATIONS __pragma(optimize( "", off ))
#define other beam_other
#include <boost/context/continuation.hpp>
#undef other
#else
#define BEAM_DISABLE_OPTIMIZATIONS
#include <boost/context/continuation.hpp>
#endif
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/Routines.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Utilities/ReportException.hpp"
#include "Beam/Utilities/StackPrint.hpp"

namespace Beam {
namespace Routines {

  /*! \class ScheduledRoutine
      \brief A Routine that executes within a Scheduler.
  */
  class ScheduledRoutine : public Routine {
    public:

      //! Returns the Scheduler this Routine runs through.
      Details::Scheduler& GetScheduler() const;

      //! Returns the id of the context this Routine is running in.
      std::size_t GetContextId() const;

      //! Continues execution of this Routine from its last defer point or from
      //! the beginning if it has not yet executed.
      void Continue();

    protected:

      //! Constructs a ScheduledRoutine.
      /*!
        \param contextId The id of the Context to run in, or set to the number
               of threads in the Scheduler to assign it an arbitrary context
               id.
        \param stackSize The size of the stack to allocate.
        \param scheduler The Scheduler this Routine will execute through.
      */
      ScheduledRoutine(std::size_t contextId, std::size_t stackSize,
        Ref<Details::Scheduler> scheduler);

      void Defer() override;

      void PendingSuspend() override;

      void Suspend() override;

      void Resume() override;

    private:
      friend class Details::Scheduler;
      std::size_t m_contextId;
      bool m_isPendingResume;
      Details::Scheduler* m_scheduler;
      boost::context::continuation m_continuation;
      boost::context::continuation m_parent;
      std::string m_stackPrint;

      bool IsPendingResume() const;
      void SetPendingResume(bool value);
      boost::context::continuation InitializeRoutine(
        boost::context::continuation&& parent);
  };

  inline Details::Scheduler& ScheduledRoutine::GetScheduler() const {
    return *m_scheduler;
  }

  inline std::size_t ScheduledRoutine::GetContextId() const {
    return m_contextId;
  }

  inline void ScheduledRoutine::Continue() {
    Details::CurrentRoutineGlobal<void>::GetInstance() = this;
    m_isPendingResume = false;
    if(GetState() == State::PENDING) {
      SetState(State::RUNNING);
      m_continuation = boost::context::callcc(
        [=] (boost::context::continuation&& parent) {
          return InitializeRoutine(std::move(parent));
        });
    } else {
      SetState(State::RUNNING);
      m_continuation = m_continuation.resume();
    }
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
  }

  inline ScheduledRoutine::ScheduledRoutine(std::size_t contextId,
      std::size_t stackSize, Ref<Details::Scheduler> scheduler)
      : m_scheduler{scheduler.Get()},
        m_isPendingResume{false} {
    if(contextId == boost::thread::hardware_concurrency()) {
      m_contextId = GetId() % boost::thread::hardware_concurrency();
    } else {
      m_contextId = contextId;
    }
  }

  BEAM_DISABLE_OPTIMIZATIONS
  inline void ScheduledRoutine::Defer() {
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
    #ifdef BEAM_ENABLE_STACK_PRINT
    #ifndef NDEBUG
    m_stackPrint = CaptureStackPrint();
    #endif
    #endif
    m_parent = m_parent.resume();
  }

  inline void ScheduledRoutine::PendingSuspend() {
    SetState(State::PENDING_SUSPEND);
  }

  inline void ScheduledRoutine::Suspend() {
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
    SetState(State::PENDING_SUSPEND);
    #ifdef BEAM_ENABLE_STACK_PRINT
    #ifndef NDEBUG
    m_stackPrint = CaptureStackPrint();
    #endif
    #endif
    m_parent = m_parent.resume();
  }

  inline bool ScheduledRoutine::IsPendingResume() const {
    return m_isPendingResume;
  }

  inline void ScheduledRoutine::SetPendingResume(bool value) {
    m_isPendingResume = value;
  }

  inline boost::context::continuation ScheduledRoutine::InitializeRoutine(
      boost::context::continuation&& parent) {
    m_parent = std::move(parent);
    try {
      Execute();
    } catch(const boost::context::detail::forced_unwind&) {
      throw;
    } catch(...) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
    }
    SetState(State::COMPLETE);
    return std::move(m_parent);
  }
}
}

#endif
