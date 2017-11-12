#ifndef BEAM_SWITCH_REACTOR_HPP
#define BEAM_SWITCH_REACTOR_HPP
#include <memory>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {

  /*! \class SwitchReactor
      \brief A Reactor that produces values by switching between Reactors.
      \tparam ProducerReactorType The Reactor that produces the Reactors to
              switch between.
   */
  template<typename ProducerReactorType>
  class SwitchReactor : public Reactor<
      GetReactorType<GetReactorType<ProducerReactorType>>> {
    public:
      using Type = GetReactorType<GetReactorType<ProducerReactorType>>;

      //! Constructs a SwitchReactor.
      /*!
        \param producer The Reactor that produces the Reactors to switch
               between.
      */
      template<typename ProducerReactorForward>
      SwitchReactor(ProducerReactorForward&& producer);

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      using ChildReactor = GetReactorType<ProducerReactorType>;
      GetOptionalLocalPtr<ProducerReactorType> m_producer;
      boost::optional<ChildReactor> m_reactor;
      Expect<Type> m_value;
      bool m_isProducerComplete;
      int m_currentSequenceNumber;
      BaseReactor::Update m_update;
      BaseReactor::Update m_state;
  };

  //! Builds a SwitchReactor.
  /*!
    \param producer The Reactor that produces the Reactors to switch between.
  */
  template<typename Producer>
  auto MakeSwitchReactor(Producer&& producer) {
    auto producerReactor = Lift(std::forward<Producer>(producer));
    return std::make_shared<SwitchReactor<
      typename std::decay<decltype(producerReactor)>::type>>(
      std::forward<decltype(producerReactor)>(producerReactor));
  }

  //! Builds a SwitchReactor.
  /*!
    \param producer The Reactor that produces the Reactors to switch between.
  */
  template<typename Producer>
  auto Switch(Producer&& producer) {
    return MakeSwitchReactor(std::forward<Producer>(producer));
  }

  template<typename ProducerReactorType>
  template<typename ProducerReactorForward>
  SwitchReactor<ProducerReactorType>::SwitchReactor(
      ProducerReactorForward&& producer)
      : m_producer{std::forward<ProducerReactorForward>(producer)},
        m_value{std::make_exception_ptr(ReactorUnavailableException{})},
        m_isProducerComplete{false},
        m_currentSequenceNumber{-1},
        m_state{BaseReactor::Update::NONE} {}

  template<typename ProducerReactorType>
  BaseReactor::Update SwitchReactor<ProducerReactorType>::Commit(
      int sequenceNumber) {
    if(m_currentSequenceNumber == sequenceNumber) {
      return m_update;
    } else if(sequenceNumber == 0 && m_currentSequenceNumber != -1) {
      return m_state;
    } else if(IsComplete(m_state)) {
      return BaseReactor::Update::NONE;
    }
    auto producerUpdate = [&] {
      if(m_isProducerComplete) {
        return BaseReactor::Update::NONE;
      }
      auto update = m_producer->Commit(sequenceNumber);
      m_isProducerComplete = IsComplete(update);
      return update;
    }();
    auto update = BaseReactor::Update::NONE;
    if(HasEval(producerUpdate)) {
      try {
        auto reactor = m_producer->Eval();
        auto reactorUpdate = reactor->Commit(0);
        if(HasEval(reactorUpdate)) {
          m_value = TryEval(*reactor);
          update = BaseReactor::Update::EVAL;
        }
        if(!IsComplete(reactorUpdate)) {
          m_reactor.emplace(std::move(reactor));
        }
      } catch(const std::exception&) {
        m_value = std::current_exception();
        update = BaseReactor::Update::EVAL;
      }
    }
    if(m_reactor.is_initialized()) {
      auto reactorUpdate = (*m_reactor)->Commit(sequenceNumber);
      if(update == BaseReactor::Update::NONE && HasEval(reactorUpdate)) {
        m_value = TryEval(**m_reactor);
        update = BaseReactor::Update::EVAL;
      }
      if(IsComplete(update)) {
        m_reactor.reset();
      }
    }
    if(m_isProducerComplete && !m_reactor.is_initialized()) {
      Combine(update, BaseReactor::Update::COMPLETE);
    }
    m_currentSequenceNumber = sequenceNumber;
    m_update = update;
    Combine(m_state, update);
    return update;
  }

  template<typename ProducerReactorType>
  typename SwitchReactor<ProducerReactorType>::Type
      SwitchReactor<ProducerReactorType>::Eval() const {
    return m_value.Get();
  }
}
}

#endif
