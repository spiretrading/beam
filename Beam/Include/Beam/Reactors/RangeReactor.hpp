#ifndef BEAM_RANGEREACTOR_HPP
#define BEAM_RANGEREACTOR_HPP
#include <tuple>
#include <utility>
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/TriggeredReactor.hpp"

namespace Beam {
namespace Reactors {
  template<typename ResultType>
  struct RangeReactorCore {
    using Result = ResultType;
    bool m_isInitialized;
    std::shared_ptr<TriggeredReactor<Result>> m_trigger;

    RangeReactorCore()
        : m_isInitialized(false),
          m_trigger(std::make_shared<TriggeredReactor<Result>>()) {
      m_trigger->SetValue(Result());
      m_trigger->Trigger();
      m_trigger->Execute();
    }

    Result operator ()(const Result& lower, const Result& upper,
        const Result& last) {
      if(m_isInitialized) {
        if(last >= upper) {
          return last;
        }
        if(last + 1 == upper) {
          m_trigger->SetComplete();
        } else {
          m_trigger->SetValue(last + 1);
        }
        m_trigger->Trigger();
        return last + 1;
      }
      m_isInitialized = true;
      m_trigger->SetValue(lower);
      m_trigger->Trigger();
      return lower;
    }
  };

  //! Builds a Reactor that produces a range of values.
  /*!
    \param lower The Reactor producing the first value in the range.
    \param upper The Reactor producing the last value in the range.
  */
  template<typename LowerReactor, typename UpperReactor>
  std::tuple<std::shared_ptr<Reactor<GetReactorType<LowerReactor>>>,
      std::shared_ptr<Event>> MakeRangeReactor(LowerReactor&& lower,
      UpperReactor&& upper) {
    RangeReactorCore<GetReactorType<LowerReactor>> core;
    auto reactor = MakeFunctionReactor(core, std::forward<LowerReactor>(lower),
      std::forward<UpperReactor>(upper), core.m_trigger);
    return std::make_tuple(
      std::static_pointer_cast<Reactor<GetReactorType<LowerReactor>>>(reactor),
      std::static_pointer_cast<Event>(core.m_trigger));
  }
}
}

#endif
