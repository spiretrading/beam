#ifndef BEAM_REACTORS_EXPRESSIONS_HPP
#define BEAM_REACTORS_EXPRESSIONS_HPP
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  //! Makes a Reactor that adds two Reactors together.
  /*!
    \param lhs The left hand side to add.
    \param rhs The right hand side to add.
  */
  template<typename T, typename U>
  auto Add(std::shared_ptr<Reactor<T>> lhs, std::shared_ptr<Reactor<U>> rhs) {
    return MakeFunctionReactor(
      [] (const T& lhs, const U& rhs) {
        return lhs + rhs;
      }, std::move(lhs), std::move(rhs));
  }
}
}

#endif
