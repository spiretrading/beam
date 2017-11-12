#ifndef BEAM_AGGREGATE_REACTOR_HPP
#define BEAM_AGGREGATE_REACTOR_HPP
#include <utility>
#include <vector>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"

namespace Beam {
namespace Reactors {

  /*! \class AggregateReactor
      \brief A Reactor that aggregates multiple Reactors together.
      \tparam ProducerReactorType The Reactor that produces the Reactors to
              switch between.
   */
  template<typename ProducerReactorType>
  class AggregateReactor : public Reactor<GetReactorType<
      GetReactorType<ProducerReactorType>>>{
    public:
      using Type = GetReactorType<GetReactorType<ProducerReactorType>>;

      //! Constructs an AggregateReactor.
      /*!
        \param producer The Reactor that produces the Reactors to aggregate.
      */
      template<typename ProducerReactorForward>
      AggregateReactor(ProducerReactorForward&& producer);

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      using ChildReactor = GetReactorType<ProducerReactorType>;
      GetOptionalLocalPtr<ProducerReactorType> m_producer;
      std::vector<ChildReactor> m_children;
      Expect<Type> m_value;
      bool m_isProducerComplete;
      int m_currentSequenceNumber;
      BaseReactor::Update m_update;
      BaseReactor::Update m_state;
  };

  //! Makes an AggregateReactor.
  /*!
    \param producer The Reactor that produces the Reactors to aggregate.
  */
  template<typename ProducerReactor>
  auto MakeAggregateReactor(ProducerReactor&& producer) {
    return std::make_shared<AggregateReactor<
      typename std::decay<ProducerReactor>::type>>(
      std::forward<ProducerReactor>(producer));
  }

  template<typename ProducerReactorType>
  template<typename ProducerReactorForward>
  AggregateReactor<ProducerReactorType>::AggregateReactor(
      ProducerReactorForward&& producer)
      : m_producer{std::forward<ProducerReactorForward>(producer)},
        m_value{std::make_exception_ptr(ReactorUnavailableException{})},
        m_isProducerComplete{false},
        m_currentSequenceNumber{-1},
        m_state{BaseReactor::Update::NONE} {}

  template<typename ProducerReactorType>
  BaseReactor::Update AggregateReactor<ProducerReactorType>::Commit(
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
        auto child = m_producer->Eval();
        auto childUpdate = child->Commit(0);
        if(HasEval(childUpdate)) {
          m_value = TryEval(*child);
          update = BaseReactor::Update::EVAL;
        }
        if(!IsComplete(childUpdate)) {
          m_children.push_back(std::move(child));
        }
      } catch(const std::exception&) {
        m_value = std::current_exception();
        update = BaseReactor::Update::EVAL;
      }
    }
    m_children.erase(std::remove_if(m_children.begin(), m_children.end(),
      [&] (auto& child) {
        auto childUpdate = child->Commit(sequenceNumber);
        if(update == BaseReactor::Update::NONE && HasEval(childUpdate)) {
          m_value = TryEval(*child);
          update = BaseReactor::Update::EVAL;
        }
        return IsComplete(childUpdate);
      }), m_children.end());
    if(m_isProducerComplete && m_children.empty()) {
      Combine(update, BaseReactor::Update::COMPLETE);
    }
    m_currentSequenceNumber = sequenceNumber;
    m_update = update;
    Combine(m_state, update);
    return update;
  }

  template<typename ProducerReactorType>
  typename AggregateReactor<ProducerReactorType>::Type
      AggregateReactor<ProducerReactorType>::Eval() const {
    return m_value.Get();
  }
}
}

#endif
