#ifndef BEAM_FILTERREACTOR_HPP
#define BEAM_FILTERREACTOR_HPP
#include <utility>
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {
  template<typename ResultType>
  struct FilterReactorCore {
    using Result = ResultType;

    boost::optional<Result> operator ()(bool filter, const Result& value) {
      if(filter) {
        return value;
      }
      return boost::none;
    }
  };

  //! Builds a Reactor that filters out values.
  /*!
    \param filter The filter to apply.
    \param source Produces the values.
  */
  template<typename FilterReactor, typename SourceReactor>
  std::shared_ptr<Reactor<GetReactorType<SourceReactor>>> MakeFilterReactor(
      FilterReactor&& filter, SourceReactor&& source) {
    return MakeFunctionReactor(
      FilterReactorCore<GetReactorType<SourceReactor>>(),
      std::forward<FilterReactor>(filter), std::forward<SourceReactor>(source));
  }
}
}

#endif
