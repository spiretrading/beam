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

      virtual bool IsComplete() const override final;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      using ChildReactor = GetReactorType<ProducerReactorType>;
      GetOptionalLocalPtr<ProducerReactorType> m_producer;
      std::vector<ChildReactor> m_children;
      int m_currentSequenceNumber;
      BaseReactor::Update m_update;
      Expect<Type> m_value;
      bool m_hasValue;
      bool m_isProducerComplete;
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
        m_currentSequenceNumber{-1},
        m_value{std::make_exception_ptr(ReactorUnavailableException{})},
        m_hasValue{false},
        m_isProducerComplete{false} {}

  template<typename ProducerReactorType>
  bool AggregateReactor<ProducerReactorType>::IsComplete() const {
    return m_isProducerComplete && m_children.empty();
  }

  template<typename ProducerReactorType>
  BaseReactor::Update AggregateReactor<ProducerReactorType>::Commit(
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
    auto producerCommit = [&] {
      if(m_isProducerComplete) {
        return BaseReactor::Update::NONE;
      }
      return m_producer->Commit(sequenceNumber);
    }();
    if(producerCommit == BaseReactor::Update::EVAL) {
      try {
        auto child = m_producer->Eval();
        auto childCommit = child->Commit(0);
        if(childCommit == BaseReactor::Update::NONE) {
          m_children.push_back(std::move(child));
        } else if(childCommit == BaseReactor::Update::EVAL) {
          m_value = Try(
            [&] {
              return child->Eval();
            });
          m_children.push_back(std::move(child));
          m_update = BaseReactor::Update::EVAL;
          m_hasValue = true;
          m_currentSequenceNumber = sequenceNumber;
          return m_update;
        }
      } catch(const std::exception&) {
        m_value = std::exception_ptr();
        m_update = BaseReactor::Update::EVAL;
        m_hasValue = true;
        m_currentSequenceNumber = sequenceNumber;
        return m_update;
      }
      m_isProducerComplete = m_producer->IsComplete();
    } else if(producerCommit == BaseReactor::Update::COMPLETE) {
      m_isProducerComplete = true;
    }
    auto i = m_children.begin();
    while(i != m_children.end()) {
      auto& child = *i;
      auto childCommit = child->Commit(sequenceNumber);
      if(childCommit == BaseReactor::Update::EVAL) {
        m_value = Try(
          [&] {
            return child->Eval();
          });
        m_update = BaseReactor::Update::EVAL;
        m_hasValue = true;
        m_currentSequenceNumber = sequenceNumber;
        if(child->IsComplete()) {
          m_children.erase(i);
        }
        return m_update;
      } else if(childCommit == BaseReactor::Update::COMPLETE) {
        i = m_children.erase(i);
      } else {
        ++i;
      }
    }
    return BaseReactor::Update::NONE;
  }

  template<typename ProducerReactorType>
  typename AggregateReactor<ProducerReactorType>::Type
      AggregateReactor<ProducerReactorType>::Eval() const {
    return m_value.Get();
  }
}
}

#endif
