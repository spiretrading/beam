#ifndef BEAM_SCHEDULEDROUTINE_HPP
#define BEAM_SCHEDULEDROUTINE_HPP
#include <iostream>
#include <boost/context/detail/fcontext.hpp>
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/Routines.hpp"
#include "Beam/Routines/simple_stack_allocator.hpp"
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
      virtual ~ScheduledRoutine();

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
               of threads in the Scheduler to assign it an arbitrary context id.
        \param stackSize The size of the stack to allocate.
        \param scheduler The Scheduler this Routine will execute through.
      */
      ScheduledRoutine(std::size_t contextId, std::size_t stackSize,
        RefType<Details::Scheduler> scheduler);

      virtual void Execute() = 0;

      virtual void Defer();

      virtual void PendingSuspend();

      virtual void Suspend();

      virtual void Resume();

    private:
      friend class Details::Scheduler;
      std::size_t m_contextId;
      bool m_isPendingResume;
      Details::Scheduler* m_scheduler;
      std::size_t m_stackSize;
      void* m_stackPointer;
      boost::context::detail::fcontext_t m_parentContext;
      boost::context::detail::fcontext_t m_context;
      std::string m_stackPrint;

      bool IsPendingResume() const;
      void SetPendingResume(bool value);

      static void InitializeRoutine(boost::context::detail::transfer_t r);
  };

  inline ScheduledRoutine::~ScheduledRoutine() {
    boost::context::simple_stack_allocator<8 * 1024 * 1024, 64 * 1024, 1024>
      allocator;
    allocator.deallocate(m_stackPointer, m_stackSize);
  }

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
      m_context = boost::context::detail::make_fcontext(m_stackPointer,
        m_stackSize, ScheduledRoutine::InitializeRoutine);
      m_context = boost::context::detail::jump_fcontext(m_context, this).fctx;
    } else {
      SetState(State::RUNNING);
      m_context = boost::context::detail::jump_fcontext(
        m_context, nullptr).fctx;
    }
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
  }

  inline ScheduledRoutine::ScheduledRoutine(std::size_t contextId,
      std::size_t stackSize, RefType<Details::Scheduler> scheduler)
      : m_stackSize{stackSize},
        m_scheduler{scheduler.Get()},
        m_isPendingResume{false} {
    if(contextId == boost::thread::hardware_concurrency()) {
      m_contextId = GetId() % boost::thread::hardware_concurrency();
    } else {
      m_contextId = contextId;
    }
    boost::context::simple_stack_allocator<8 * 1024 * 1024, 64 * 1024, 1024>
      allocator;
    m_stackPointer = allocator.allocate(m_stackSize);
  }

  inline void ScheduledRoutine::Defer() {
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
#ifdef BEAM_ENABLE_STACK_PRINT
#ifndef NDEBUG
    m_stackPrint = CaptureStackPrint();
#endif
#endif
    m_parentContext = boost::context::detail::jump_fcontext(
      m_parentContext, nullptr).fctx;
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
    m_parentContext = boost::context::detail::jump_fcontext(
      m_parentContext, nullptr).fctx;
  }

  inline bool ScheduledRoutine::IsPendingResume() const {
    return m_isPendingResume;
  }

  inline void ScheduledRoutine::SetPendingResume(bool value) {
    m_isPendingResume = value;
  }

  inline void ScheduledRoutine::InitializeRoutine(
      boost::context::detail::transfer_t r) {
    auto routine = reinterpret_cast<ScheduledRoutine*>(r.data);
    routine->m_parentContext = r.fctx;
    try {
      routine->Execute();
    } catch(...) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
    }
    routine->SetState(State::COMPLETE);
    routine->Defer();
  }
}
}

#endif
