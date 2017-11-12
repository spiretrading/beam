#ifndef BEAM_PROXY_REACTOR_HPP
#define BEAM_PROXY_REACTOR_HPP
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"

namespace Beam {
namespace Reactors {

  /*! \class ProxyReactor
      \brief A Reactor that evaluates to another Reactor.
      \tparam T The type of value this Reactor evaluates to.
   */
  template<typename T>
  class ProxyReactor : public Reactor<T> {
    public:
      using Type = T;

      //! Constructs a ProxyReactor.
      ProxyReactor();

      //! Constructs a ProxyReactor.
      /*!
        \param reactor The Reactor to reference.
      */
      ProxyReactor(std::shared_ptr<Reactor<T>> reactor);

      //! Sets the Reactor to reference.
      /*!
        \param reactor The Reactor to reference.
      */
      void SetReactor(std::shared_ptr<Reactor<T>> reactor);

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      std::weak_ptr<Reactor<T>> m_reactor;
      mutable bool m_cycle;
  };

  //! Makes a ProxyReactor.
  template<typename T>
  auto MakeProxyReactor() {
    return std::make_shared<ProxyReactor<T>>();
  }

  //! Makes a ProxyReactor.
  /*!
    \param reactor The Reactor to reference.
  */
  template<typename ReactorForward>
  auto MakeProxyReactor(ReactorForward reactor) {
    return std::make_shared<ProxyReactor<GetReactorType<ReactorForward>>>(
      std::move(reactor));
  }

  template<typename T>
  ProxyReactor<T>::ProxyReactor()
      : m_cycle{false} {}

  template<typename T>
  ProxyReactor<T>::ProxyReactor(std::shared_ptr<Reactor<T>> reactor)
      : m_reactor{std::move(reactor)},
        m_cycle{false} {}

  template<typename T>
  void ProxyReactor<T>::SetReactor(std::shared_ptr<Reactor<T>> reactor) {
    m_reactor = std::move(reactor);
  }

  template<typename T>
  BaseReactor::Update ProxyReactor<T>::Commit(int sequenceNumber) {
    if(m_cycle) {
      return BaseReactor::Update::NONE;
    }
    auto reactor = m_reactor.lock();
    if(reactor == nullptr) {
      return BaseReactor::Update::NONE;
    }
    try {
      m_cycle = true;
      auto update = reactor->Commit(sequenceNumber);
      m_cycle = false;
      return update;
    } catch(const std::exception&) {
      m_cycle = false;
      throw;
    }
  }

  template<typename T>
  typename ProxyReactor<T>::Type ProxyReactor<T>::Eval() const {
    auto reactor = m_reactor.lock();
    if(reactor == nullptr) {
      BOOST_THROW_EXCEPTION(ReactorUnavailableException{});
    }
    return reactor->Eval();
  }
}
}

#endif
