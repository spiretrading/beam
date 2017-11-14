#ifndef BEAM_FIRST_REACTOR_HPP
#define BEAM_FIRST_REACTOR_HPP
#include <utility>
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  //! Makes a Reactor that evaluates to the first value it receives from its
  //! source.
  /*!
    \param source The source that will provide the value to evaluate to.
  */
  template<typename Source>
  auto MakeFirstReactor(Source&& source) {
    return MakeFunctionReactor(
      [] (auto&& value) {
        return MakeFunctionEvaluation(std::forward<decltype(value)>(value),
          BaseReactor::Update::COMPLETE);
      }, Lift(std::forward<Source>(source)));
  }

  //! Makes a Reactor that evaluates to the first value it receives from its
  //! source.
  /*!
    \param source The source that will provide the value to evaluate to.
  */
  template<typename Source>
  auto First(Source&& source) {
    return MakeFirstReactor(std::forward<Source>(source));
  }
}
}

#endif
