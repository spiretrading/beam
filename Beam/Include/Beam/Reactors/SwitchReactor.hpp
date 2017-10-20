#ifndef BEAM_SWITCH_REACTOR_HPP
#define BEAM_SWITCH_REACTOR_HPP
#include <memory>
#include <utility>
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"

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
      : m_producer{std::forward<ProducerReactorForward>(producer)} {}

  template<typename ProducerReactorType>
  bool SwitchReactor<ProducerReactorType>::IsComplete() const {
    return false;
  }

  template<typename ProducerReactorType>
  BaseReactor::Update SwitchReactor<ProducerReactorType>::Commit(
      int sequenceNumber) {
    return BaseReactor::Update::NONE;
  }

  template<typename ProducerReactorType>
  typename SwitchReactor<ProducerReactorType>::Type
      SwitchReactor<ProducerReactorType>::Eval() const {
    BOOST_THROW_EXCEPTION(ReactorUnavailableException{});
  }
}
}

#endif
