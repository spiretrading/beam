#ifndef BEAM_CHAIN_REACTOR_HPP
#define BEAM_CHAIN_REACTOR_HPP
#include <type_traits>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/Trigger.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {

  /*! \class ChainReactor
      \brief A Reactor that produces values from one Reactor until it completes
             and then produces values from another Reactor.
      \tparam InitialReactorType The Reactor to initially evaluate to.
      \tparam ContinuationReactorType The Reactor to evaluate to thereafter.
   */
  template<typename InitialReactorType, typename ContinuationReactorType>
  class ChainReactor : public Reactor<GetReactorType<InitialReactorType>> {
    public:
      using Type = GetReactorType<InitialReactorType>;

      //! Constructs a ChainReactor.
      /*!
        \param initialReactor The Reactor to initially evaluate to.
        \param continuationReactor The Reactor to evaluate to thereafter.
      */
      template<typename InitialReactorForward,
        typename ContinuationReactorForward>
      ChainReactor(InitialReactorForward&& initialReactor,
        ContinuationReactorForward&& continuationReactor);

      virtual bool IsComplete() const override final;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      enum class State {
        INITIAL,
        TRANSITION,
        CONTINUATION,
      };
      GetOptionalLocalPtr<InitialReactorType> m_initialReactor;
      GetOptionalLocalPtr<ContinuationReactorType> m_continuationReactor;
      const Reactor<Type>* m_currentReactor;
      int m_transitionSequence;
      State m_state;
  };

  //! Makes a ChainReactor.
  /*!
    \param initial The Reactor to initially evaluate to.
    \param continuation The Reactor to evaluate to thereafter.
  */
  template<typename Initial, typename Continuation>
  auto MakeChainReactor(Initial&& initial, Continuation&& continuation) {
    auto initialReactor = Lift(std::forward<Initial>(initial));
    auto continuationReactor = Lift(std::forward<Continuation>(continuation));
    return std::make_shared<ChainReactor<
      typename std::decay<decltype(initialReactor)>::type,
      typename std::decay<decltype(continuationReactor)>::type>>(
      std::forward<decltype(initialReactor)>(initialReactor),
      std::forward<decltype(continuationReactor)>(continuationReactor));
  }

  //! Makes a ChainReactor.
  /*!
    \param initial The Reactor to initially evaluate to.
    \param continuation The Reactor to evaluate to thereafter.
  */
  template<typename Initial, typename Continuation>
  auto Chain(Initial&& initial, Continuation&& continuation) {
    return MakeChainReactor(std::forward<Initial>(initial),
      std::forward<Continuation>(continuation));
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  template<typename InitialReactorForward, typename ContinuationReactorForward>
  ChainReactor<InitialReactorType, ContinuationReactorType>::ChainReactor(
      InitialReactorForward&& initialReactor,
      ContinuationReactorForward&& continuationReactor)
      : m_initialReactor{std::forward<InitialReactorForward>(initialReactor)},
        m_continuationReactor{
          std::forward<ContinuationReactorForward>(continuationReactor)},
        m_currentReactor{&*m_initialReactor},
        m_transitionSequence{-1},
        m_state{State::INITIAL} {}

  template<typename InitialReactorType, typename ContinuationReactorType>
  bool ChainReactor<InitialReactorType, ContinuationReactorType>::
      IsComplete() const {
    return m_currentReactor == &*m_continuationReactor &&
      m_currentReactor->IsComplete();
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  BaseReactor::Update ChainReactor<InitialReactorType,
      ContinuationReactorType>::Commit(int sequenceNumber) {
    if(m_state == State::INITIAL) {
      auto update = m_initialReactor->Commit(sequenceNumber);
      if(update == BaseReactor::Update::COMPLETE) {
        m_currentReactor = &*m_continuationReactor;
        m_state = State::CONTINUATION;
        return m_continuationReactor->Commit(0);
      } else if(update == BaseReactor::Update::EVAL &&
          m_initialReactor->IsComplete()) {
        m_state = State::TRANSITION;
        Trigger::GetEnvironmentTrigger().SignalUpdate(
          Store(m_transitionSequence));
      }
      return update;
    } else if(m_state == State::TRANSITION) {
      if(sequenceNumber == m_transitionSequence) {
        m_currentReactor = &*m_continuationReactor;
        m_state = State::CONTINUATION;
        return m_continuationReactor->Commit(0);
      }
      return BaseReactor::Update::NONE;
    } else {
      return m_continuationReactor->Commit(sequenceNumber);
    }
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  typename ChainReactor<InitialReactorType, ContinuationReactorType>::Type
      ChainReactor<InitialReactorType, ContinuationReactorType>::Eval() const {
    return m_currentReactor->Eval();
  }
}
}

#endif
