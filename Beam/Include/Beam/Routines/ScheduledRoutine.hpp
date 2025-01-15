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

namespace Beam::Routines {

  /* Implements a Routine that executes within a Scheduler. */
  class ScheduledRoutine : public Routine {
    public:

      /** Returns the id of the context this Routine is running in. */
      std::size_t GetContextId() const;

      /**
       * Continues execution of this Routine from its last defer point or from
       * the beginning if it has not yet executed.
       */
      void Continue();

    protected:

      /**
       * Constructs a ScheduledRoutine.
       * @param stackSize The size of the stack to allocate.
       * @param contextId The id of the Context to run in, or -1 to assign it an
       *        arbitrary context id.
       */
      ScheduledRoutine(std::size_t stackSize, std::size_t contextId);

      void Defer() override;

      void PendingSuspend() override;

      void Suspend() override;

      void Resume() override;

    private:
      friend class Details::Scheduler;
      bool m_isPendingResume;
      std::size_t m_stackSize;
      std::size_t m_contextId;
      boost::context::continuation m_continuation;
      boost::context::continuation m_parent;
      #ifndef NDEBUG
      std::string m_stackPrint;
      #endif

      bool IsPendingResume() const;
      void SetPendingResume(bool value);
      boost::context::continuation InitializeRoutine(
        boost::context::continuation&& parent);
  };

  inline std::size_t ScheduledRoutine::GetContextId() const {
    return m_contextId;
  }

  inline void ScheduledRoutine::Continue() {
    Details::CurrentRoutineGlobal<void>::GetInstance() = this;
    m_isPendingResume = false;
    if(GetState() == State::PENDING) {
      SetState(State::RUNNING);
      m_continuation = boost::context::callcc(std::allocator_arg,
        boost::context::fixedsize_stack(m_stackSize),
        [this] (boost::context::continuation&& parent) {
          Details::CurrentRoutineGlobal<void>::isInsideRoutine = true;
          ++Details::CurrentRoutineGlobal<void>::activeRoutineCount;
          return InitializeRoutine(std::move(parent));
        });
    } else {
      SetState(State::RUNNING);
      Details::CurrentRoutineGlobal<void>::isInsideRoutine = true;
      ++Details::CurrentRoutineGlobal<void>::activeRoutineCount;
      m_continuation = m_continuation.resume();
    }
    Details::CurrentRoutineGlobal<void>::isInsideRoutine = false;
    --Details::CurrentRoutineGlobal<void>::activeRoutineCount;
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
  }

  inline ScheduledRoutine::ScheduledRoutine(std::size_t stackSize,
      std::size_t contextId)
      : m_isPendingResume(false),
        m_stackSize(stackSize) {
    if(contextId == -1) {
      m_contextId = GetId() % 2;
    } else {
      m_contextId = contextId;
    }
  }

  BEAM_DISABLE_OPTIMIZATIONS
  inline void ScheduledRoutine::Defer() {
    Details::CurrentRoutineGlobal<void>::isInsideRoutine = false;
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
    Details::CurrentRoutineGlobal<void>::isInsideRoutine = false;
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

#endif
