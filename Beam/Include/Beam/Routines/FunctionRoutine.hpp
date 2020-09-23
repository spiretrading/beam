#ifndef BEAM_FUNCTION_ROUTINE_HPP
#define BEAM_FUNCTION_ROUTINE_HPP
#include <boost/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Routines/ScheduledRoutine.hpp"

namespace Beam::Routines {

  /**
   * Implements a Routine using a callable object.
   * @param <F> The type of the callable object to run.
   */
  template<typename F>
  class FunctionRoutine final : public ScheduledRoutine {
    public:

      /**
       * Constructs a FunctionRoutine.
       * @param function The callable object to run.
       * @param contextId The id of the Context to run in, or -1 to assign it an
       *        arbitrary context id.
       * @param stackSize The size of the stack to allocate.
       */
      template<typename FF>
      FunctionRoutine(FF&& function, std::size_t stackSize,
        std::size_t contextId);

    protected:
      void Execute() override;

    private:
      boost::optional<F> m_function;
  };

  template<typename F>
  FunctionRoutine(F&&, std::size_t, std::size_t) ->
    FunctionRoutine<std::remove_reference_t<F>>;

  template<typename F>
  template<typename FF>
  FunctionRoutine<F>::FunctionRoutine(FF&& function, std::size_t stackSize,
    std::size_t contextId)
    : ScheduledRoutine(stackSize, contextId),
      m_function(std::forward<FF>(function)) {}

  template<typename F>
  void FunctionRoutine<F>::Execute() {
    try {
      (*m_function)();
    } catch(...) {
      m_function = boost::none;
      throw;
    }
    m_function = boost::none;
  }
}

#endif
