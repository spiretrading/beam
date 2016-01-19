#ifndef BEAM_CHAINREACTOR_HPP
#define BEAM_CHAINREACTOR_HPP
#include <deque>
#include <type_traits>
#include "Beam/Reactors/Event.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/ReactorContainer.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
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
  class ChainReactor : public Reactor<GetReactorType<InitialReactorType>>,
      public Event {
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

      virtual void Commit();

      virtual Type Eval() const;

      virtual void Execute();

    private:
      ReactorContainer<InitialReactorType> m_initialReactor;
      ReactorContainer<ContinuationReactorType> m_continuationReactor;
      std::deque<Expect<Type>> m_continuationValues;
      Expect<Type> m_value;
      int m_state;

      void S0(bool initialUpdated, bool continuationUpdated);
      void S1(bool initialUpdated);
      void S2(bool continuationUpdated);
      void S3(bool continuationUpdated);
      void S4();
      void S5(bool continuationUpdated);
      void S6();
      void S7(bool continuationUpdated);
      void S8();
  };

  //! Makes a ChainReactor.
  /*!
    \param initialReactor The Reactor to initially evaluate to.
    \param continuationReactor The Reactor to evaluate to thereafter.
  */
  template<typename InitialReactor, typename ContinuationReactor>
  std::shared_ptr<ChainReactor<typename std::decay<InitialReactor>::type,
      typename std::decay<ContinuationReactor>::type>>
      MakeChainReactor(InitialReactor&& initialReactor,
      ContinuationReactor&& continuationReactor) {
    return std::make_shared<ChainReactor<
      typename std::decay<InitialReactor>::type,
      typename std::decay<ContinuationReactor>::type>>(
      std::forward<InitialReactor>(initialReactor),
      std::forward<ContinuationReactor>(continuationReactor));
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  template<typename InitialReactorForward, typename ContinuationReactorForward>
  ChainReactor<InitialReactorType, ContinuationReactorType>::ChainReactor(
      InitialReactorForward&& initialReactor,
      ContinuationReactorForward&& continuationReactor)
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_initialReactor(std::forward<InitialReactorForward>(initialReactor),
          *this),
        m_continuationReactor(std::forward<ContinuationReactorForward>(
          continuationReactor), *this),
        m_state(0) {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

  template<typename InitialReactorType, typename ContinuationReactorType>
  void ChainReactor<InitialReactorType, ContinuationReactorType>::Commit() {
    auto initialUpdated = m_initialReactor.Commit();
    auto continuationUpdated = m_continuationReactor.Commit();
    if(m_state == 0) {
      return S0(initialUpdated, continuationUpdated);
    } else if(m_state == 4) {
      return S5(continuationUpdated);
    } else if(m_state == 6) {
      return S7(continuationUpdated);
    }
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  typename ChainReactor<InitialReactorType, ContinuationReactorType>::Type
      ChainReactor<InitialReactorType, ContinuationReactorType>::Eval() const {
    return m_value.Get();
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  void ChainReactor<InitialReactorType, ContinuationReactorType>::Execute() {
    this->SignalUpdate();
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  void ChainReactor<InitialReactorType, ContinuationReactorType>::S0(
      bool initialUpdated, bool continuationUpdated) {
    m_state = 0;
    if(continuationUpdated) {

      // C0
      return S1(initialUpdated);
    } else if(initialUpdated) {

      // C1
      return S2(continuationUpdated);
    } else if(m_initialReactor.IsComplete()) {

      // C2
      return S3(continuationUpdated);
    }
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  void ChainReactor<InitialReactorType, ContinuationReactorType>::S1(
      bool initialUpdated) {
    m_state = 1;
    m_continuationValues.push_back(m_continuationReactor.GetValue());
    return S0(initialUpdated, false);
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  void ChainReactor<InitialReactorType, ContinuationReactorType>::S2(
      bool continuationUpdated) {
    m_state = 2;
    m_value = m_initialReactor.GetValue();
    this->IncrementSequenceNumber();
    return S0(false, continuationUpdated);
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  void ChainReactor<InitialReactorType, ContinuationReactorType>::S3(
      bool continuationUpdated) {
    m_state = 3;
    if(this->IsInitializing() && !m_continuationValues.empty()) {

      // C3
      return S5(continuationUpdated);
    } else if(!m_continuationValues.empty()) {

      // C4
      return S4();
    } else if(m_continuationValues.empty()) {

      // ~C4
      return S6();
    }
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  void ChainReactor<InitialReactorType, ContinuationReactorType>::S4() {
    m_state = 4;
    this->SignalEvent();
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  void ChainReactor<InitialReactorType, ContinuationReactorType>::S5(
      bool continuationUpdated) {
    m_state = 5;
    if(continuationUpdated) {
      m_continuationValues.push_back(m_continuationReactor.GetValue());
    }
    m_value = std::move(m_continuationValues.front());
    this->IncrementSequenceNumber();
    m_continuationValues.pop_front();
    return S3(false);
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  void ChainReactor<InitialReactorType, ContinuationReactorType>::S6() {
    m_state = 6;
    if(m_continuationReactor.IsComplete()) {

      // C5
      return S8();
    }
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  void ChainReactor<InitialReactorType, ContinuationReactorType>::S7(
      bool continuationUpdated) {
    m_state = 7;
    if(continuationUpdated) {
      m_value = m_continuationReactor.GetValue();
      this->IncrementSequenceNumber();
    }
    if(m_continuationReactor.IsComplete()) {

      // C5
      return S8();
    } else {

      // ~C5
      return S6();
    }
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  void ChainReactor<InitialReactorType, ContinuationReactorType>::S8() {
    m_state = 8;
    this->SetComplete();
  }
}
}

#endif
