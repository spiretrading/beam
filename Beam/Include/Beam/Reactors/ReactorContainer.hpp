#ifndef BEAM_REACTORCONTAINER_HPP
#define BEAM_REACTORCONTAINER_HPP
#include <boost/noncopyable.hpp>
#include <boost/signals2/connection.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {
namespace Details {
  template<typename T, typename Reactor>
  void AssignReactor(Expect<T>& value, Reactor& reactor) {
    value.Try(
      [&] {
        return reactor->Eval();
      });
  }

  template<typename Reactor>
  void AssignReactor(Expect<void>& value, Reactor& reactor) {
    value = reactor->GetBaseValue();
  }

  template<typename T>
  struct EvalType {
    typedef const T& type;
  };

  template<>
  struct EvalType<void> {
    typedef void type;
  };
}

  /*! \class ReactorContainer
      \brief Stores a snapshot of a Reactor's most recently observed state.
      \tparam ReactorType The type of Reactor to contain.
    */
  template<typename ReactorType>
  class ReactorContainer : private boost::noncopyable {
    public:

      //! The type of Reactor to container.
      typedef GetTryDereferenceType<ReactorType> Reactor;

      //! The type that the Reactor evaluates to.
      typedef GetReactorType<ReactorType> Type;

      //! The type returned when evaluated.
      typedef typename Details::EvalType<Type>::type EvalType;

      //! Constructs a ReactorContainer.
      /*!
        \param reactor Initializes the Reactor.
        \param updateSlot The slot that receives updates from the
               <i>reactor</i>.
      */
      template<typename ReactorForward>
      ReactorContainer(ReactorForward&& expression,
        const BaseReactor::UpdateSignal::slot_type& updateSlot);

      //! Constructs a ReactorContainer.
      /*!
        \param reactor Initializes the Reactor.
        \param parent The parent Reactor.
      */
      template<typename ReactorForward>
      ReactorContainer(ReactorForward&& expression, BaseReactor& parent);

      //! Returns the Reactor.
      const Reactor& GetReactor() const;

      //! Returns <code>true</code> iff the Reactor is initializing.
      bool IsInitializing() const;

      //! Returns <code>true</code> iff the Reactor is initialized.
      bool IsInitialized() const;

      //! Returns <code>true</code> iff the Reactor is complete.
      bool IsComplete() const;

      //! Returns the Reactor's value.
      const Expect<Type>& GetValue() const;

      //! Returns the Reactor's value.
      EvalType Eval() const;

      //! Commits changes to the Reactor and updates the snapshot.
      /*!
        \return <code>true</code> iff the Reactor's sequence number was updated.
      */
      bool Commit();

    private:
      GetOptionalLocalPtr<ReactorType> m_reactor;
      boost::signals2::scoped_connection m_connection;
      unsigned int m_sequenceNumber;
      bool m_isComplete;
      Expect<Type> m_value;
  };

  template<typename ReactorType>
  template<typename ReactorForward>
  ReactorContainer<ReactorType>::ReactorContainer(ReactorForward&& reactor,
      const BaseReactor::UpdateSignal::slot_type& updateSlot)
      : m_reactor(std::forward<ReactorForward>(reactor)),
        m_connection(m_reactor->ConnectUpdateSignal(updateSlot)),
        m_sequenceNumber(0),
        m_isComplete(false) {}

  template<typename ReactorType>
  template<typename ReactorForward>
  ReactorContainer<ReactorType>::ReactorContainer(ReactorForward&& reactor,
      BaseReactor& parent)
      : m_reactor(std::forward<ReactorForward>(reactor)),
        m_connection(m_reactor->ConnectUpdateSignal(
          std::bind(&BaseReactor::SignalUpdate, &parent))),
        m_sequenceNumber(0),
        m_isComplete(false) {}

  template<typename ReactorType>
  const typename ReactorContainer<ReactorType>::Reactor&
      ReactorContainer<ReactorType>::GetReactor() const {
    return *m_reactor;
  }

  template<typename ReactorType>
  bool ReactorContainer<ReactorType>::IsInitializing() const {
    return !IsInitialized();
  }

  template<typename ReactorType>
  bool ReactorContainer<ReactorType>::IsInitialized() const {
    return m_sequenceNumber != 0 || m_isComplete;
  }

  template<typename ReactorType>
  bool ReactorContainer<ReactorType>::IsComplete() const {
    return m_isComplete;
  }

  template<typename ReactorType>
  const Expect<typename ReactorContainer<ReactorType>::Type>&
      ReactorContainer<ReactorType>::GetValue() const {
    return m_value;
  }

  template<typename ReactorType>
  typename ReactorContainer<ReactorType>::EvalType
      ReactorContainer<ReactorType>::Eval() const {
    return m_value.Get();
  }

  template<typename ReactorType>
  bool ReactorContainer<ReactorType>::Commit() {
    if(m_isComplete) {
      return false;
    }
    if(m_sequenceNumber == m_reactor->GetSequenceNumber()) {
      m_reactor->Commit();
    }
    auto hasChange = false;
    if(m_sequenceNumber != m_reactor->GetSequenceNumber()) {
      Details::AssignReactor(m_value, m_reactor);
      m_sequenceNumber = m_reactor->GetSequenceNumber();
      hasChange = true;
    }
    m_isComplete = m_reactor->IsComplete();
    return hasChange;
  }
}
}

#endif
