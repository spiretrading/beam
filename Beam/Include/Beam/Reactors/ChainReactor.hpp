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

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      enum class ChainState {
        INITIAL,
        TRANSITION,
        CONTINUATION,
      };
      GetOptionalLocalPtr<InitialReactorType> m_initialReactor;
      GetOptionalLocalPtr<ContinuationReactorType> m_continuationReactor;
      const Reactor<Type>* m_currentReactor;
      int m_transitionSequence;
      ChainState m_chainState;
      int m_currentSequenceNumber;
      BaseReactor::Update m_update;
      BaseReactor::Update m_state;
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
        m_chainState{ChainState::INITIAL},
        m_currentSequenceNumber{-1},
        m_state{BaseReactor::Update::NONE} {}

  template<typename InitialReactorType, typename ContinuationReactorType>
  BaseReactor::Update ChainReactor<InitialReactorType,
      ContinuationReactorType>::Commit(int sequenceNumber) {
    if(m_currentSequenceNumber == sequenceNumber) {
      return m_update;
    } else if(sequenceNumber == 0 && m_currentSequenceNumber != -1) {
      return m_state;
    } else if(IsComplete(m_state)) {
      return BaseReactor::Update::NONE;
    }
    if(m_chainState == ChainState::INITIAL) {
      auto update = m_initialReactor->Commit(sequenceNumber);
      if(update == BaseReactor::Update::COMPLETE) {
        m_chainState = ChainState::CONTINUATION;
        m_currentSequenceNumber = sequenceNumber;
        m_update = m_continuationReactor->Commit(0);
        if(m_update != BaseReactor::Update::COMPLETE) {
          m_currentReactor = &*m_continuationReactor;
        }
        Combine(m_state, m_update);
        return m_update;
      } else if(update == BaseReactor::Update::COMPLETE_WITH_EVAL) {
        m_chainState = ChainState::TRANSITION;
        Trigger::GetEnvironmentTrigger().SignalUpdate(
          Store(m_transitionSequence));
        m_currentSequenceNumber = sequenceNumber;
        m_update = BaseReactor::Update::EVAL;
        Combine(m_state, BaseReactor::Update::EVAL);
        return BaseReactor::Update::EVAL;
      }
      m_currentSequenceNumber = sequenceNumber;
      m_update = update;
      Combine(m_state, update);
      return update;
    } else if(m_chainState == ChainState::TRANSITION) {
      if(sequenceNumber == m_transitionSequence) {
        m_chainState = ChainState::CONTINUATION;
        m_currentSequenceNumber = sequenceNumber;
        m_update = m_continuationReactor->Commit(0);
        if(HasEval(m_update)) {
          m_currentReactor = &*m_continuationReactor;
        }
        Combine(m_state, m_update);
        return m_update;
      } else {
        return BaseReactor::Update::NONE;
      }
    } else {
      m_currentSequenceNumber = sequenceNumber;
      m_update = m_continuationReactor->Commit(sequenceNumber);
      if(HasEval(m_update)) {
        m_currentReactor = &*m_continuationReactor;
      }
      Combine(m_state, m_update);
      return m_update;
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
