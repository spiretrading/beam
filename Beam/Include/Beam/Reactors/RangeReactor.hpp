#ifndef BEAM_RANGE_REACTOR_HPP
#define BEAM_RANGE_REACTOR_HPP
#include <utility>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Reactors/BasicReactor.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/Trigger.hpp"

namespace Beam {
namespace Reactors {
namespace Details {
  template<typename ResultType>
  struct RangeReactorCore {
    using Result = ResultType;
    bool m_isInitialized;
    std::shared_ptr<BasicReactor<Result>> m_iterator;

    RangeReactorCore(RefType<Trigger> trigger)
        : m_isInitialized{false},
          m_iterator{std::make_shared<BasicReactor<Result>>(Ref(trigger))} {
      m_iterator->Update(Result{});
    }

    boost::optional<Result> operator ()(const Result& lower,
        const Result& upper, const Result& last) {
      if(m_isInitialized) {
        if(last >= upper) {
          return boost::none;
        }
        if(last + 1 == upper) {
          m_iterator->SetComplete();
        } else {
          m_iterator->Update(last + 1);
        }
        return last + 1;
      }
      m_isInitialized = true;
      m_iterator->Update(lower);
      return lower;
    }
  };
}

  //! Builds a Reactor that produces a range of values.
  /*!
    \param lower The Reactor producing the first value in the range.
    \param upper The Reactor producing the last value in the range.
    \param trigger The Trigger used to indicate an update.
  */
  template<typename LowerReactor, typename UpperReactor>
  auto MakeRangeReactor(LowerReactor&& lower, UpperReactor&& upper,
      RefType<Trigger> trigger) {
    Details::RangeReactorCore<GetReactorType<LowerReactor>> core{Ref(trigger)};
    return MakeFunctionReactor(core, std::forward<LowerReactor>(lower),
      std::forward<UpperReactor>(upper), core.m_iterator);
  }
}
}

#endif
