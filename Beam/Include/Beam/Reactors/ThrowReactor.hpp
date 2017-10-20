#ifndef BEAM_THROW_REACTOR_HPP
#define BEAM_THROW_REACTOR_HPP
#include <exception>
#include <utility>
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/ReactorException.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  //! Makes a Reactor that throws a ReactorException.
  /*!
    \param exception The ReactorException to throw.
  */
  template<typename T>
  auto MakeThrowReactor(const ReactorException& exception) {
    auto e = std::make_exception_ptr(exception);
    return MakeFunctionReactor(
      [=] () -> T {
        std::rethrow_exception(e);
        return *static_cast<T*>(nullptr);
      });
  }
}
}

#endif
