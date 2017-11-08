#ifndef BEAM_DO_REACTOR_HPP
#define BEAM_DO_REACTOR_HPP
#include <memory>
#include <utility>
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {
namespace Details {
  template<typename Function>
  struct DoFunction {
    mutable Function m_function;

    template<typename FunctionForward>
    DoFunction(FunctionForward&& function)
        : m_function{std::forward<FunctionForward>(function)} {}

    template<typename T>
    const T& operator ()(const Expect<T>& value) const {
      m_function(value);
      return value.Get();
    }

    void operator ()(const Expect<void>& value) const {
      m_function(value);
      value.Get();
    }
  };
}

  //! Builds a Reactor that performs a side-effect when its parameter updates.
  /*!
    \param function The function performing the side-effect.
    \param reactor The Reactor producing the updates.
  */
  template<typename Function, typename Reactor>
  auto Do(Function&& function, Reactor&& reactor) {
    return MakeFunctionReactor(
      Details::DoFunction<typename std::decay<Function>::type>{
      std::forward<Function>(function)}, Lift(std::forward<Reactor>(reactor)));
  }
}
}

#endif
