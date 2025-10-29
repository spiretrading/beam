#ifndef BEAM_SCHEDULED_ROUTINE_HPP
#define BEAM_SCHEDULED_ROUTINE_HPP
#include <iostream>
#ifdef WIN32
  #define BOOST_USE_WINFIB
#endif
#if defined _MSC_VER
  #define BEAM_DISABLE_OPTIMIZATIONS __pragma(optimize( "", off ))
  #define other beam_other
  #include <boost/context/continuation.hpp>
  #undef other
#else
  #define BEAM_DISABLE_OPTIMIZATIONS
  #include <boost/context/continuation.hpp>
#endif
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Routines/Routine.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {
namespace Details {
  class Scheduler;
}

  /* Implements a Routine that executes within a Scheduler. */
  class ScheduledRoutine : public Routine {
    public:

      /** Returns the id of the context this Routine is running in. */
      std::size_t get_context_id() const;

      /**
       * Continues execution of this Routine from its last defer point or from
       * the beginning if it has not yet executed.
       */
      void advance();

    protected:

      /**
       * Constructs a ScheduledRoutine.
       * @param stack_size The size of the stack to allocate.
       * @param context_id The id of the Context to run in or -1 to assign it an
       *        arbitrary context id.
       */
      ScheduledRoutine(std::size_t stack_size, std::size_t context_id) noexcept;

      void defer() override;
      void pending_suspend() override;
      void suspend() override;
      void resume() override;

    private:
      friend class Details::Scheduler;
      bool m_is_pending_resume;
      std::size_t m_stack_size;
      std::size_t m_context_id;
      boost::context::continuation m_continuation;
      boost::context::continuation m_parent;

      bool is_pending_resume() const;
      void set_pending_resume(bool value);
      boost::context::continuation initialize(
        boost::context::continuation&& parent);
  };

  inline std::size_t ScheduledRoutine::get_context_id() const {
    return m_context_id;
  }

  inline void ScheduledRoutine::advance() {
    Details::CurrentRoutineGlobal::get() = this;
    m_is_pending_resume = false;
    if(get_state() == State::PENDING) {
      set(State::RUNNING);
      m_continuation = boost::context::callcc(std::allocator_arg,
        boost::context::fixedsize_stack(m_stack_size),
        [this] (boost::context::continuation&& parent) {
          return initialize(std::move(parent));
        });
    } else {
      set(State::RUNNING);
      m_continuation = m_continuation.resume();
    }
    Details::CurrentRoutineGlobal::get() = nullptr;
  }

  inline ScheduledRoutine::ScheduledRoutine(
      std::size_t stack_size, std::size_t context_id) noexcept
      : m_is_pending_resume(false),
        m_stack_size(stack_size) {
    if(context_id == -1) {
      m_context_id = get_id() % boost::thread::hardware_concurrency();
    } else {
      m_context_id = context_id;
    }
  }

  BEAM_DISABLE_OPTIMIZATIONS
  inline void ScheduledRoutine::defer() {
    Details::CurrentRoutineGlobal::get() = nullptr;
    m_parent = m_parent.resume();
  }

  inline void ScheduledRoutine::pending_suspend() {
    set(State::PENDING_SUSPEND);
  }

  inline void ScheduledRoutine::suspend() {
    Details::CurrentRoutineGlobal::get() = nullptr;
    set(State::PENDING_SUSPEND);
    m_parent = m_parent.resume();
  }

  inline bool ScheduledRoutine::is_pending_resume() const {
    return m_is_pending_resume;
  }

  inline void ScheduledRoutine::set_pending_resume(bool value) {
    m_is_pending_resume = value;
  }

  inline boost::context::continuation ScheduledRoutine::initialize(
      boost::context::continuation&& parent) {
    m_parent = std::move(parent);
    try {
      execute();
    } catch(const boost::context::detail::forced_unwind&) {
      throw;
    } catch(...) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
    }
    set(State::COMPLETE);
    return std::move(m_parent);
  }
}

#endif
