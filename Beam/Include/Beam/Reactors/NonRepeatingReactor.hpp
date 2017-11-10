#ifndef BEAM_NON_REPEATING_REACTOR_HPP
#define BEAM_NON_REPEATING_REACTOR_HPP
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/Trigger.hpp"

namespace Beam {
namespace Reactors {
namespace Details {
  template<typename T>
  struct NonRepeatingCore {
    using Type = T;
    boost::optional<T> m_lastValue;

    boost::optional<Type> operator ()(const Type& value) {
      if(m_lastValue.is_initialized()) {
        if(*m_lastValue == value) {
          return boost::none;
        }
      }
      m_lastValue = value;
      return value;
    }
  };
}

  //! Makes a Reactor that never repeats the same value twice in a row.
  /*!
    \param child The Reactor to wrap.
  */
  template<typename Child>
  auto MakeNonRepeatingReactor(Child&& child) {
    auto childReactor = Lift(std::forward<Child>(child));
    return MakeFunctionReactor(
      Details::NonRepeatingCore<GetReactorType<decltype(childReactor)>>{},
      std::forward<decltype(childReactor)>(childReactor));
  }
}
}

#endif
