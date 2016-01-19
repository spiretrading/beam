#ifndef BEAM_NONREPEATINGREACTOR_HPP
#define BEAM_NONREPEATINGREACTOR_HPP
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {
  template<typename ResultType>
  struct NonRepeatingReactorCore {
    using Result = ResultType;
    boost::optional<Result> m_previous;

    boost::optional<Result> operator ()(const Result& value) {
      if(m_previous == value) {
        return boost::none;
      }
      m_previous = value;
      return m_previous;
    }
  };

  //! Builds a Reactor that does not produce the same value successively.
  /*!
    \param reactor The Reactor producing a value.
  */
  template<typename SourceReactor>
  std::shared_ptr<Reactor<GetReactorType<SourceReactor>>>
      MakeNonRepeatingReactor(SourceReactor&& source) {
    auto reactor = MakeFunctionReactor(
      NonRepeatingReactorCore<GetReactorType<SourceReactor>>(),
      std::forward<SourceReactor>(source));
    return reactor;
  }
}
}

#endif
