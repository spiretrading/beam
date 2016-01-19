#ifndef BEAM_AGGREGATEREACTOR_HPP
#define BEAM_AGGREGATEREACTOR_HPP
#include <utility>
#include <vector>
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/ReactorContainer.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/Expect.hpp"

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
      typedef GetReactorType<GetReactorType<ProducerReactorType>> Type;

      //! Constructs an AggregateReactor.
      /*!
        \param producer The Reactor that produces the Reactors to aggregate.
      */
      template<typename ProducerReactorForward>
      AggregateReactor(ProducerReactorForward&& producer);

      virtual void Commit();

      virtual Type Eval() const;

    private:
      struct Conditions {
        bool m_producerHasUpdate;
        bool m_hasPendingValue;
        bool m_producerIsComplete;
      };
      typedef GetReactorType<ProducerReactorType> Reactor;
      ReactorContainer<ProducerReactorType> m_producer;
      std::vector<std::unique_ptr<ReactorContainer<Reactor>>> m_reactors;
      int m_state;
      Expect<Type> m_value;

      void S0(Conditions& conditions);
      void S1(Conditions& conditions);
      void S2(Conditions& conditions);
      void S3();
  };

  //! Makes an AggregateReactor.
  /*!
    \param producer The Reactor that produces the Reactors to aggregate.
  */
  template<typename ProducerReactor>
  std::shared_ptr<AggregateReactor<typename std::decay<ProducerReactor>::type>>
      MakeAggregateReactor(ProducerReactor&& producer) {
    return std::make_shared<AggregateReactor<
      typename std::decay<ProducerReactor>::type>>(
      std::forward<ProducerReactor>(producer));
  }

  template<typename ProducerReactorType>
  template<typename ProducerReactorForward>
  AggregateReactor<ProducerReactorType>::AggregateReactor(
      ProducerReactorForward&& producer)
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_producer(std::forward<ProducerReactorForward>(producer), *this),
        m_state(0) {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

  template<typename ProducerReactorType>
  void AggregateReactor<ProducerReactorType>::Commit() {
    Conditions conditions;
    conditions.m_producerHasUpdate = m_producer.Commit();
    conditions.m_producerIsComplete = true;
    conditions.m_hasPendingValue = false;
    for(auto& reactor : m_reactors) {
      if(reactor->Commit() && !conditions.m_hasPendingValue) {
        m_value = reactor->GetValue();
        conditions.m_hasPendingValue = true;
      }
      conditions.m_producerIsComplete &= reactor->IsComplete();
    }
    conditions.m_producerIsComplete &= m_producer.IsComplete();
    return S0(conditions);
  }

  template<typename ProducerReactorType>
  typename AggregateReactor<ProducerReactorType>::Type
      AggregateReactor<ProducerReactorType>::Eval() const {
    return m_value.Get();
  }

  template<typename ProducerReactorType>
  void AggregateReactor<ProducerReactorType>::S0(Conditions& conditions) {
    m_state = 0;
    if(conditions.m_producerHasUpdate) {

      // C0
      return S1(conditions);
    } else if(conditions.m_hasPendingValue) {

      // C1
      return S2(conditions);
    } else if(conditions.m_producerIsComplete) {

      // C2
      return S3();
    }
  }

  template<typename ProducerReactorType>
  void AggregateReactor<ProducerReactorType>::S1(Conditions& conditions) {
    m_state = 1;
    auto reactor = std::make_unique<ReactorContainer<Reactor>>(
      m_producer.Eval(), *this);
    conditions.m_producerHasUpdate = false;
    if(reactor->Commit() && !conditions.m_hasPendingValue) {
      m_value = reactor->GetValue();
      conditions.m_hasPendingValue = true;
    }
    m_reactors.push_back(std::move(reactor));
    return S0(conditions);
  }

  template<typename ProducerReactorType>
  void AggregateReactor<ProducerReactorType>::S2(Conditions& conditions) {
    m_state = 2;
    this->IncrementSequenceNumber();
    conditions.m_hasPendingValue = false;
    return S0(conditions);
  }

  template<typename ProducerReactorType>
  void AggregateReactor<ProducerReactorType>::S3() {
    m_state = 3;
    this->SetComplete();
  }
}
}

#endif
