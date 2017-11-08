#ifndef BEAM_FILTER_REACTOR_HPP
#define BEAM_FILTER_REACTOR_HPP
#include <utility>
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {
namespace Details {
  struct FilterReactorCore {
    template<typename T>
    boost::optional<T> operator ()(bool filter, const Expect<T>& value) const {
      if(filter) {
        return value.Get();
      }
      return boost::none;
    }
  };
}

  //! Builds a Reactor that filters out values.
  /*!
    \param filter The filter to apply.
    \param source Produces the values.
  */
  template<typename FilterReactor, typename SourceReactor>
  auto MakeFilterReactor(FilterReactor&& filter, SourceReactor&& source) {
    return MakeFunctionReactor(Details::FilterReactorCore{},
      std::forward<FilterReactor>(filter),
      Lift(std::forward<SourceReactor>(source)));
  }

  //! Builds a Reactor that filters out values.
  /*!
    \param filter The filter to apply.
    \param source Produces the values.
  */
  template<typename FilterReactor, typename SourceReactor>
  auto Filter(FilterReactor&& filter, SourceReactor&& source) {
    return MakeFilterReactor(std::forward<FilterReactor>(filter),
      std::forward<SourceReactor>(source));
  }
}
}

#endif
