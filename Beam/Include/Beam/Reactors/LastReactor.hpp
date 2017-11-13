#ifndef BEAM_LAST_REACTOR_HPP
#define BEAM_LAST_REACTOR_HPP
#include <utility>
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/UpdateReactor.hpp"

namespace Beam {
namespace Reactors {
namespace Details {
  template<typename T>
  struct LastReactorCore {
    using Type = T;
    boost::optional<Type> m_lastValue;

    boost::optional<Type> operator ()(const Type& value,
        BaseReactor::Update update) {
      if(update == BaseReactor::Update::COMPLETE_WITH_EVAL) {
        return value;
      } else if(update == BaseReactor::Update::NONE) {
        return boost::none;
      } else if(HasEval(update)) {
        m_lastValue = value;
        return boost::none;
      } else {
        return m_lastValue;
      }
    }
  };
}

  //! Makes a Reactor that evaluates to the last value it receives from its
  //! source.
  /*!
    \param source The source that will provide the value to evaluate to.
  */
  template<typename Source>
  auto MakeLastReactor(Source&& source) {
    auto sourceReactor = Lift(std::forward<Source>(source));
    auto updateReactor = MakeUpdateReactor(sourceReactor);
    return MakeFunctionReactor(
      Details::LastReactorCore<GetReactorType<decltype(sourceReactor)>>{},
      std::forward<decltype(sourceReactor)>(sourceReactor),
      std::move(updateReactor));
  }

  //! Makes a Reactor that evaluates to the last value it receives from its
  //! source.
  /*!
    \param source The source that will provide the value to evaluate to.
  */
  template<typename Source>
  auto Last(Source&& source) {
    return MakeLastReactor(std::forward<Source>(source));
  }
}
}

#endif
