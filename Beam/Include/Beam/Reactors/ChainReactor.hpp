#ifndef BEAM_CHAIN_REACTOR_HPP
#define BEAM_CHAIN_REACTOR_HPP
#include <type_traits>
#include "Beam/Pointers/LocalPtr.hpp"
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
        \param trigger Used to indicate a transition from the initial Reactor to
               the continuation Reactor.
        \param initialReactor The Reactor to initially evaluate to.
        \param continuationReactor The Reactor to evaluate to thereafter.
      */
      template<typename InitialReactorForward,
        typename ContinuationReactorForward>
      ChainReactor(RefType<Trigger> trigger,
        InitialReactorForward&& initialReactor,
        ContinuationReactorForward&& continuationReactor);

      virtual bool IsComplete() const override final;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      enum class State {
        INITIAL,
        TRANSITION,
        CONTINUATION,
        COMPLETE
      };
      Trigger* m_trigger;
      GetOptionalLocalPtr<InitialReactorType> m_initialReactor;
      GetOptionalLocalPtr<ContinuationReactorType> m_continuationReactor;
      State m_state;
      int m_transitionSequenceNumber;
      int m_currentSequenceNumber;
      BaseReactor::Update m_update;
      Expect<Type> m_value;
      bool m_hasValue;
  };

  //! Makes a ChainReactor.
  /*!
    \param trigger Used to indicate a transition from the initial Reactor to
            the continuation Reactor.
    \param initialReactor The Reactor to initially evaluate to.
    \param continuationReactor The Reactor to evaluate to thereafter.
  */
  template<typename InitialReactor, typename ContinuationReactor>
  auto MakeChainReactor(RefType<Trigger> trigger,
      InitialReactor&& initialReactor,
      ContinuationReactor&& continuationReactor) {
    return std::make_shared<ChainReactor<
      typename std::decay<InitialReactor>::type,
      typename std::decay<ContinuationReactor>::type>>(Ref(trigger),
      std::forward<InitialReactor>(initialReactor),
      std::forward<ContinuationReactor>(continuationReactor));
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  template<typename InitialReactorForward, typename ContinuationReactorForward>
  ChainReactor<InitialReactorType, ContinuationReactorType>::ChainReactor(
      RefType<Trigger> trigger, InitialReactorForward&& initialReactor,
      ContinuationReactorForward&& continuationReactor)
      : m_trigger{trigger.Get()},
        m_initialReactor{std::forward<InitialReactorForward>(initialReactor)},
        m_continuationReactor{
          std::forward<ContinuationReactorForward>(continuationReactor)},
        m_state{State::INITIAL},
        m_currentSequenceNumber{-1},
        m_value{std::make_exception_ptr(ReactorUnavailableException{})},
        m_hasValue{false} {}

  template<typename InitialReactorType, typename ContinuationReactorType>
  bool ChainReactor<InitialReactorType, ContinuationReactorType>::
      IsComplete() const {
    return m_state == State::COMPLETE;
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  BaseReactor::Update ChainReactor<InitialReactorType,
      ContinuationReactorType>::Commit(int sequenceNumber) {
    if(sequenceNumber == m_currentSequenceNumber) {
      return m_update;
    } else if(sequenceNumber == 0 && m_currentSequenceNumber != -1) {
      if(m_hasValue) {
        return BaseReactor::Update::EVAL;
      }
      return BaseReactor::Update::COMPLETE;
    }
    if(m_state == State::INITIAL) {
      auto update = m_initialReactor->Commit(sequenceNumber);
      if(update == BaseReactor::Update::NONE) {
        return update;
      } else if(update == BaseReactor::Update::EVAL) {
        m_value = Try(
          [&] {
            return m_initialReactor->Eval();
          });
        if(m_initialReactor->IsComplete()) {
          m_trigger->SignalUpdate(Store(m_transitionSequenceNumber));
          m_state = State::TRANSITION;
        }
        m_currentSequenceNumber = sequenceNumber;
        m_update = BaseReactor::Update::EVAL;
        m_hasValue = true;
        return m_update;
      } else {
        m_state = State::TRANSITION;
        m_transitionSequenceNumber = sequenceNumber;
      }
    }
    if(m_state == State::TRANSITION || m_state == State::CONTINUATION) {
      auto commitSequence =
        [&] {
          if(sequenceNumber == m_transitionSequenceNumber) {
            return 0;
          } else {
            return sequenceNumber;
          }
        }();
      auto update = m_continuationReactor->Commit(commitSequence);
      if(update == BaseReactor::Update::NONE) {
        return update;
      } else if(update == BaseReactor::Update::EVAL) {
        m_value = Try(
          [&] {
            return m_continuationReactor->Eval();
          });
        m_update = BaseReactor::Update::EVAL;
        m_hasValue = true;
      } else {
        m_update = BaseReactor::Update::COMPLETE;
      }
      if(m_continuationReactor->IsComplete()) {
        m_state = State::COMPLETE;
      }
      m_currentSequenceNumber = sequenceNumber;
      return m_update;
    }
    return BaseReactor::Update::NONE;
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  typename ChainReactor<InitialReactorType, ContinuationReactorType>::Type
      ChainReactor<InitialReactorType, ContinuationReactorType>::Eval() const {
    return m_value.Get();
  }
}
}

#endif
