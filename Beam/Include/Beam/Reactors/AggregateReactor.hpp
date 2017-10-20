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

      virtual bool IsComplete() const override;

      virtual BaseReactor::Update Commit(int sequenceNumber) override;

      virtual Type Eval() const override;

    private:
      using ChildReactor = GetReactorType<ProducerReactorType>;
      GetOptionalLocalPtr<ProducerReactorType> m_producer;
      std::vector<ChildReactor> m_children;
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
      : m_producer{std::forward<ProducerReactorForward>(producer)} {}

  template<typename ProducerReactorType>
  bool AggregateReactor<ProducerReactorType>::IsComplete() const {
    return false;
  }

  template<typename ProducerReactorType>
  BaseReactor::Update AggregateReactor<ProducerReactorType>::Commit(
      int sequenceNumber) {
    return BaseReactor::Update::NONE;
  }

  template<typename ProducerReactorType>
  typename AggregateReactor<ProducerReactorType>::Type
      AggregateReactor<ProducerReactorType>::Eval() const {
    BOOST_THROW_EXCEPTION(ReactorUnavailableException{});
  }
}
}

#endif
