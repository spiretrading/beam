#ifndef BEAM_STATIC_REACTOR_HPP
#define BEAM_STATIC_REACTOR_HPP
#include <utility>
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {
namespace Details {
  struct StaticReactorCore {
    bool m_isExpired;

    StaticReactorCore()
        : m_isExpired{false} {}

    template<typename T>
    FunctionEvaluation<T> operator ()(const Expect<T>& value) {
      if(m_isExpired) {
        return boost::none;
      }
      m_isExpired = true;
      return FunctionEvaluation<T>{value, BaseReactor::Update::COMPLETE};
    }
  };
}

  //! Makes a Reactor that evaluates to the first value it receives from its
  //! source.
  /*!
    \param source The source that will provide the value to evaluate to.
  */
  template<typename ReactorType>
  auto MakeStaticReactor(ReactorType&& source) {
    return MakeFunctionReactor(Details::StaticReactorCore{},
      std::forward<ReactorType>(source));
  }
}
}

#endif
