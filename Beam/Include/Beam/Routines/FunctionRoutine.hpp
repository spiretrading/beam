#ifndef BEAM_FUNCTIONROUTINE_HPP
#define BEAM_FUNCTIONROUTINE_HPP
#include <boost/throw_exception.hpp>
#include "Beam/Pointers/DelayPtr.hpp"
#include "Beam/Routines/ScheduledRoutine.hpp"

namespace Beam {
namespace Routines {

  /*! \class FunctionRoutine
      \brief Implements a Routine using a callable object.
      \tparam F The type of the callable object to run.
   */
  template<typename F>
  class FunctionRoutine : public ScheduledRoutine {
    public:

      //! Constructs a FunctionRoutine.
      /*!
        \param function The callable object to run.
        \param contextId The id of the Context to run in, or set to the number
               of threads in the Scheduler to assign it an arbitrary context id.
        \param stackSize The size of the stack to allocate.
        \param scheduler The Scheduler this Routine runs through.
      */
      template<typename FunctionForward>
      FunctionRoutine(FunctionForward&& function, std::size_t contextId,
        std::size_t stackSize, RefType<Details::Scheduler> scheduler);

    protected:
      virtual void Execute();

    private:
      DelayPtr<F> m_function;
  };

  template<typename F>
  template<typename FunctionForward>
  FunctionRoutine<F>::FunctionRoutine(FunctionForward&& function,
      std::size_t contextId, std::size_t stackSize,
      RefType<Details::Scheduler> scheduler)
      : ScheduledRoutine{contextId, stackSize, Ref(scheduler)},
          m_function{std::forward<FunctionForward>(function)} {}

  template<typename F>
  void FunctionRoutine<F>::Execute() {
    try {
      (*m_function)();
    } catch(...) {
      m_function.Reset();
      throw;
    }
    m_function.Reset();
  }
}
}

#endif
