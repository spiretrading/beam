#ifndef BEAM_SWITCH_REACTOR_HPP
#define BEAM_SWITCH_REACTOR_HPP
#include <memory>
#include <utility>
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

      virtual bool IsComplete() const override final;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      using ChildReactor = GetReactorType<ProducerReactorType>;
      GetOptionalLocalPtr<ProducerReactorType> m_producer;
      ChildReactor m_reactor;
      BaseReactor::Update m_state;
      int m_currentSequenceNumber;
      BaseReactor::Update m_update;
      Expect<Type> m_value;
      bool m_hasValue;
  };

  //! Builds a SwitchReactor.
  /*!
    \param producer The Reactor that produces the Reactors to switch between.
  */
  template<typename ProducerReactor>
  auto MakeSwitchReactor(ProducerReactor&& producer) {
    return std::make_shared<SwitchReactor<
      typename std::decay<ProducerReactor>::type>>(
      std::forward<ProducerReactor>(producer));
  }

  template<typename ProducerReactorType>
  template<typename ProducerReactorForward>
  SwitchReactor<ProducerReactorType>::SwitchReactor(
      ProducerReactorForward&& producer)
      : m_producer{std::forward<ProducerReactorForward>(producer)},
        m_state{BaseReactor::Update::NONE},
        m_currentSequenceNumber{-1},
        m_value{std::make_exception_ptr(ReactorUnavailableException{})},
        m_hasValue{false} {}

  template<typename ProducerReactorType>
  bool SwitchReactor<ProducerReactorType>::IsComplete() const {
    return m_state == BaseReactor::Update::COMPLETE;
  }

  template<typename ProducerReactorType>
  BaseReactor::Update SwitchReactor<ProducerReactorType>::Commit(
      int sequenceNumber) {
    if(sequenceNumber == m_currentSequenceNumber) {
      return m_update;
    } else if(sequenceNumber == 0 && m_currentSequenceNumber != -1) {
      if(m_hasValue) {
        return BaseReactor::Update::EVAL;
      }
      return BaseReactor::Update::COMPLETE;
    }
    if(IsComplete()) {
      return BaseReactor::Update::NONE;
    }
    auto producerCommit = m_producer->Commit(sequenceNumber);
    if(producerCommit == BaseReactor::Update::NONE ||
        producerCommit == BaseReactor::Update::COMPLETE) {
      auto childCommit = m_reactor->Commit(sequenceNumber);
      if(childCommit == BaseReactor::Update::NONE &&
          producerCommit == BaseReactor::Update::NONE) {
        return BaseReactor::Update::NONE;
      } else if(childCommit == BaseReactor::Update::EVAL) {
        m_value = Try(
          [&] {
            return m_reactor->Eval();
          });
        m_update = BaseReactor::Update::EVAL;
        m_hasValue = true;
      } else {
        if(m_producer->IsComplete()) {
          m_update = BaseReactor::Update::COMPLETE;
        } else {
          m_update = BaseReactor::Update::NONE;
        }
      }
    } else if(producerCommit == BaseReactor::Update::EVAL) {
      try {
        m_reactor = m_producer->Eval();
        auto initialCommit = m_reactor->Commit(0);
        auto childCommit = m_reactor->Commit(sequenceNumber);
        if(childCommit == BaseReactor::Update::NONE) {
          if(initialCommit == BaseReactor::Update::EVAL) {
            childCommit = BaseReactor::Update::EVAL;
          } else if(initialCommit == BaseReactor::Update::COMPLETE) {
            childCommit = BaseReactor::Update::COMPLETE;
          }
        }
        if(childCommit == BaseReactor::Update::NONE) {
          m_update = BaseReactor::Update::NONE;
        } else if(childCommit == BaseReactor::Update::EVAL) {
          m_value = Try(
            [&] {
              return m_reactor->Eval();
            });
          m_update = BaseReactor::Update::EVAL;
          m_hasValue = true;
        } else {
          if(m_producer->IsComplete()) {
            m_update = BaseReactor::Update::COMPLETE;
          } else {
            m_update = BaseReactor::Update::NONE;
          }
        }
      } catch(const std::exception&) {
        m_value = std::current_exception();
        m_update = BaseReactor::Update::EVAL;
        m_hasValue = true;
      }
    }
    if(m_reactor->IsComplete() && m_producer->IsComplete()) {
      m_state = BaseReactor::Update::COMPLETE;
    }
    m_currentSequenceNumber = sequenceNumber;
    return m_update;
  }

  template<typename ProducerReactorType>
  typename SwitchReactor<ProducerReactorType>::Type
      SwitchReactor<ProducerReactorType>::Eval() const {
    return m_value.Get();
  }
}
}

#endif
