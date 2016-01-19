#ifndef BEAM_STATICREACTOR_HPP
#define BEAM_STATICREACTOR_HPP
#include <utility>
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {
  template<typename T>
  struct StaticReactorCore {
    bool m_isExpired;

    StaticReactorCore()
        : m_isExpired(false) {}

    boost::optional<T> operator ()(const T& value) {
      if(m_isExpired) {
        return boost::none;
      }
      m_isExpired = true;
      return value;
    }
  };

  //! Makes a Reactor that evaluates to the first value it receives from its
  //! source.
  /*!
    \param source The source that will provide the value to evaluate to.
  */
  template<typename ReactorType>
  std::shared_ptr<Reactor<GetReactorType<ReactorType>>> MakeStaticReactor(
      ReactorType&& source) {
    return MakeFunctionReactor(StaticReactorCore<GetReactorType<ReactorType>>(),
      std::forward<ReactorType>(source));
  }
}
}

#endif
