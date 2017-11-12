#ifndef BEAM_REACTORS_WHEN_COMPLETE_HPP
#define BEAM_REACTORS_WHEN_COMPLETE_HPP
#include <type_traits>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  /*! \class WhenCompleteReactor
      \brief A Reactor that calls a function when its child completes.
      \tparam FunctionType The type of function to apply.
      \tparam ReactorType The type of Reactor to monitor for completion.
   */
  template<typename FunctionType, typename ReactorType>
  class WhenCompleteReactor : public Reactor<GetReactorType<ReactorType>> {
    public:
      using Type = typename Reactor<GetReactorType<ReactorType>>::Type;

      //! The type of function to apply.
      using Function = FunctionType;

      //! Constructs a WhenCompleteReactor.
      /*!
        \param function The function to apply.
        \param reactor The Reactor to monitor for completion.
      */
      template<typename FunctionForward, typename ReactorForward>
      WhenCompleteReactor(FunctionForward&& function, ReactorForward&& reactor);

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      Function m_function;
      GetOptionalLocalPtr<ReactorType> m_reactor;
      bool m_triggered;
  };

  //! Makes a WhenCompleteReactor.
  /*!
    \param function The function to apply.
    \param source The Reactor to monitor for completion.
  */
  template<typename Function, typename Source>
  auto WhenComplete(Function&& f, Source&& source) {
    auto sourceReactor = Lift(std::forward<Source>(source));
    using Reactor = decltype(*sourceReactor);
    return std::make_shared<WhenCompleteReactor<
      typename std::decay<Function>::type,
      typename std::decay<decltype(sourceReactor)>::type>>(
      std::forward<Function>(f),
      std::forward<decltype(sourceReactor)>(sourceReactor));
  }

  template<typename FunctionType, typename ReactorType>
  template<typename FunctionForward, typename ReactorForward>
  WhenCompleteReactor<FunctionType, ReactorType>::WhenCompleteReactor(
      FunctionForward&& function, ReactorForward&& reactor)
      : m_function{std::forward<FunctionForward>(function)},
        m_reactor{std::forward<ReactorForward>(reactor)},
        m_triggered{false} {}

  template<typename FunctionType, typename ReactorType>
  BaseReactor::Update WhenCompleteReactor<FunctionType, ReactorType>::Commit(
      int sequenceNumber) {
    auto update = m_reactor->Commit(sequenceNumber);
    if(IsComplete(update) && !m_triggered) {
      m_triggered = true;
      try {
        m_function();
      } catch(const std::exception&) {}
    }
    return update;
  }

  template<typename FunctionType, typename ReactorType>
  typename WhenCompleteReactor<FunctionType, ReactorType>::Type
      WhenCompleteReactor<FunctionType, ReactorType>::Eval() const {
    return m_reactor->Eval();
  }
}
}

#endif
