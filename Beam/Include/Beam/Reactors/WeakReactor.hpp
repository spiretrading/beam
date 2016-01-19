#ifndef BEAM_WEAKREACTOR_HPP
#define BEAM_WEAKREACTOR_HPP
#include <memory>
#include <boost/signals2/connection.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {

  /*! \class WeakReactor
      \brief A Reactor that holds a weak reference to another Reactor.
      \tparam ReactorType The type of Reactor to reference.
   */
  template<typename ReactorType>
  class WeakReactor : public Reactor<GetReactorType<ReactorType>> {
    public:
      typedef GetReactorType<Reactor<GetReactorType<ReactorType>>> Type;

      //! Constructs a WeakReactor.
      /*!
        \param reactor The Reactor to reference.
      */
      WeakReactor(const std::shared_ptr<ReactorType>& reactor);

      virtual void Commit();

      virtual Type Eval() const;

    private:
      std::weak_ptr<ReactorType> m_reactor;
      bool m_isAlive;
      boost::signals2::scoped_connection m_connection;
      unsigned int m_sequenceNumber;
      bool m_isComplete;
      Expect<Type> m_value;
  };

  //! Makes a WeakReactor.
  /*!
    \param reactor The Reactor to reference.
  */
  template<typename Reactor>
  std::shared_ptr<WeakReactor<Reactor>> MakeWeakReactor(
      const std::shared_ptr<Reactor>& reactor) {
    return std::make_shared<WeakReactor<Reactor>>(reactor);
  }

  template<typename ReactorType>
  WeakReactor<ReactorType>::WeakReactor(
      const std::shared_ptr<ReactorType>& reactor)
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_reactor(reactor),
        m_isAlive(true),
        m_connection(reactor->ConnectUpdateSignal(
          std::bind(&WeakReactor::SignalUpdate, this))),
        m_sequenceNumber(0),
        m_isComplete(false) {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

  template<typename ReactorType>
  void WeakReactor<ReactorType>::Commit() {
    auto reactor = m_reactor.lock();
    if(reactor != nullptr) {
      reactor->Commit();
      if(m_sequenceNumber != reactor->GetSequenceNumber()) {
        m_value.Try(
          [&] {
            return reactor->Eval();
          });
        m_sequenceNumber = reactor->GetSequenceNumber();
        this->IncrementSequenceNumber();
      }
      if(m_isComplete != reactor->IsComplete()) {
        m_isComplete = reactor->IsComplete();
        if(m_isComplete) {
          this->SetComplete();
        }
      }
    } else if(m_isAlive) {
      m_isAlive = false;
      this->SetComplete();
    }
  }

  template<typename ReactorType>
  typename WeakReactor<ReactorType>::Type WeakReactor<ReactorType>::
      Eval() const {
    return m_value.Get();
  }
}
}

#endif
