#ifndef BEAM_SWITCHREACTOR_HPP
#define BEAM_SWITCHREACTOR_HPP
#include <memory>
#include <utility>
#include "Beam/Pointers/DelayPtr.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/ReactorContainer.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {

  /*! \class SwitchReactor
      \brief A Reactor that produces values by switching between Reactors.
      \tparam ProducerReactorType The Reactor that produces the Reactors to
              switch between.
   */
  template<typename ProducerReactorType>
  class SwitchReactor : public Reactor<GetReactorType<
      GetReactorType<ProducerReactorType>>> {
    public:
      typedef GetReactorType<GetReactorType<ProducerReactorType>> Type;

      //! Constructs a SwitchReactor.
      /*!
        \param producer The Reactor that produces the Reactors to switch
               between.
      */
      template<typename ProducerReactorForward>
      SwitchReactor(ProducerReactorForward&& producer);

      virtual void Commit();

      virtual Type Eval() const;

    private:
      typedef GetReactorType<ProducerReactorType> Reactor;
      ReactorContainer<ProducerReactorType> m_producer;
      DelayPtr<ReactorContainer<Reactor>> m_reactor;
      int m_state;
      Expect<Type> m_value;

      void S0(bool producerHasUpdate);
      void S1();
      void S2();
      void S3(const Reactor& reactor);
      void S4(const std::exception_ptr& exception);
      void S5(bool producerHasUpdate, bool reactorHasUpdate);
      void S6();
      void S7(bool producerHasUpdate);
  };

  //! Builds a SwitchReactor.
  /*!
    \param producer The Reactor that produces the Reactors to switch between.
  */
  template<typename ProducerReactor>
  std::shared_ptr<SwitchReactor<typename std::decay<ProducerReactor>::type>>
      MakeSwitchReactor(ProducerReactor&& producer) {
    return std::make_shared<SwitchReactor<
      typename std::decay<ProducerReactor>::type>>(
      std::forward<ProducerReactor>(producer));
  }

  template<typename ProducerReactorType>
  template<typename ProducerReactorForward>
  SwitchReactor<ProducerReactorType>::SwitchReactor(
      ProducerReactorForward&& producer)
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_producer(std::forward<ProducerReactorForward>(producer), *this),
        m_state(0) {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

  template<typename ProducerReactorType>
  void SwitchReactor<ProducerReactorType>::Commit() {
    auto producerHasUpdate = m_producer.Commit();
    auto reactorHasUpdate = false;
    if(m_reactor.IsInitialized()) {
      reactorHasUpdate = m_reactor->Commit();
    }
    if(m_state == 0) {
      return S0(producerHasUpdate);
    } else if(m_state == 5) {
      return S5(producerHasUpdate, reactorHasUpdate);
    } else if(m_state == 7) {
      return S7(producerHasUpdate);
    }
  }

  template<typename ProducerReactorType>
  typename SwitchReactor<ProducerReactorType>::Type
      SwitchReactor<ProducerReactorType>::Eval() const {
    return m_value.Get();
  }

  template<typename ProducerReactorType>
  void SwitchReactor<ProducerReactorType>::S0(bool producerHasUpdate) {
    m_state = 0;
    if(producerHasUpdate) {

      // C0
      return S2();
    } else if(m_producer.IsComplete()) {

      // C1
      return S1();
    }
  }

  template<typename ProducerReactorType>
  void SwitchReactor<ProducerReactorType>::S1() {
    m_state = 1;
    this->SetComplete();
  }

  template<typename ProducerReactorType>
  void SwitchReactor<ProducerReactorType>::S2() {
    m_state = 2;
    m_reactor.Reset();
    Expect<Reactor> reactor;
    reactor.Try(
      [&] {
        return m_producer.Eval();
      });
    if(reactor.IsValue()) {

      // C2
      return S3(reactor.Get());
    } else {

      // ~C2
      return S4(reactor.GetException());
    }
  }

  template<typename ProducerReactorType>
  void SwitchReactor<ProducerReactorType>::S3(const Reactor& reactor) {
    m_state = 3;
    m_reactor.Initialize(reactor, *this);
    return S5(false, false);
  }

  template<typename ProducerReactorType>
  void SwitchReactor<ProducerReactorType>::S4(
      const std::exception_ptr& exception) {
    m_state = 4;
    m_value = exception;
    this->IncrementSequenceNumber();
    return S7(false);
  }

  template<typename ProducerReactorType>
  void SwitchReactor<ProducerReactorType>::S5(bool producerHasUpdate,
      bool reactorHasUpdate) {
    m_state = 5;
    if(producerHasUpdate) {

      // C0
      return S2();
    } else if(reactorHasUpdate) {

      // C3
      return S6();
    } else if(m_producer.IsComplete() && m_reactor->IsComplete()) {

      // C1 && C4
      return S1();
    }
  }

  template<typename ProducerReactorType>
  void SwitchReactor<ProducerReactorType>::S6() {
    m_state = 6;
    m_value.Try(
      [&] {
        return m_reactor->Eval();
      });
    this->IncrementSequenceNumber();
    return S5(false, false);
  }

  template<typename ProducerReactorType>
  void SwitchReactor<ProducerReactorType>::S7(bool producerHasUpdate) {
    m_state = 7;
    if(producerHasUpdate) {

      // C0
      return S2();
    } else if(m_producer.IsComplete()) {

      // C1
      return S1();
    }
  }
}
}

#endif
