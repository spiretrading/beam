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

      virtual bool IsComplete() const override final;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      Function m_function;
      GetOptionalLocalPtr<ReactorType> m_reactor;
      BaseReactor::Update m_state;
      BaseReactor::Update m_update;
      int m_currentSequenceNumber;
  };

  //! Makes a WhenCompleteReactor.
  /*!
    \param function The function to apply.
    \param reactor The Reactor to monitor for completion.
  */
  template<typename Function, typename ReactorType>
  auto WhenComplete(Function&& f, ReactorType&& reactor) {
    return std::make_shared<WhenCompleteReactor<
      typename std::decay<Function>::type,
      typename std::decay<ReactorType>::type>>(std::forward<Function>(f),
      std::forward<ReactorType>(reactor));
  }

  template<typename FunctionType, typename ReactorType>
  template<typename FunctionForward, typename ReactorForward>
  WhenCompleteReactor<FunctionType, ReactorType>::WhenCompleteReactor(
      FunctionForward&& function, ReactorForward&& reactor)
      : m_function{std::forward<FunctionForward>(function)},
        m_reactor{std::forward<ReactorForward>(reactor)},
        m_state{BaseReactor::Update::NONE},
        m_currentSequenceNumber{-1} {}

  template<typename FunctionType, typename ReactorType>
  bool WhenCompleteReactor<FunctionType, ReactorType>::IsComplete() const {
    return m_state == BaseReactor::Update::COMPLETE;
  }

  template<typename FunctionType, typename ReactorType>
  BaseReactor::Update WhenCompleteReactor<FunctionType, ReactorType>::Commit(
      int sequenceNumber) {
    if(sequenceNumber == m_currentSequenceNumber) {
      return m_update;
    }
    m_update = m_reactor->Commit(sequenceNumber);
    if(m_update == BaseReactor::Update::EVAL && m_reactor->IsComplete() ||
        m_update == BaseReactor::Update::COMPLETE) {
      try {
        m_function();
      } catch(const std::exception&) {}
    }
    m_currentSequenceNumber = sequenceNumber;
    return m_update;
  }

  template<typename FunctionType, typename ReactorType>
  typename WhenCompleteReactor<FunctionType, ReactorType>::Type
      WhenCompleteReactor<FunctionType, ReactorType>::Eval() const {
    return m_reactor->Eval();
  }
}
}

#endif
