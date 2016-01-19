#ifndef BEAM_TRIGGEREDREACTOR_HPP
#define BEAM_TRIGGEREDREACTOR_HPP
#include <type_traits>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/Reactors/Event.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/ReactorException.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {

  /*! \class TriggeredReactor
      \brief A Reactor that is programmatically updated.
   */
  template<typename T>
  class TriggeredReactor : public Reactor<T>, public Event {
    public:
      typedef GetReactorType<Reactor<T>> Type;

      //! Constructs a TriggeredReactor.
      TriggeredReactor();

      //! Sets the value.
      void SetValue(const T& value);

      //! Sets an exception.
      template<typename E>
      void SetException(const E& exception);

      //! Sets an exception.
      void SetException(const std::exception_ptr& exception);

      //! Indicates that no more updates will be made.
      void SetComplete();

      //! Triggers the next update.
      void Trigger();

      virtual void Commit();

      virtual Type Eval() const;

      virtual void Execute();

    private:
      mutable boost::mutex m_mutex;
      Expect<Type> m_value;
      boost::optional<Expect<Type>> m_nextValue;
      bool m_isPending;
      bool m_isTriggered;
      bool m_isExecuted;
      bool m_isComplete;
  };

  //! Makes a TriggeredReactor.
  template<typename T>
  std::shared_ptr<TriggeredReactor<T>> MakeTriggeredReactor() {
    return std::make_shared<TriggeredReactor<T>>();
  }

  template<typename T>
  TriggeredReactor<T>::TriggeredReactor()
      : m_isPending(false),
        m_isTriggered(false),
        m_isExecuted(false),
        m_isComplete(false) {}

  template<typename T>
  void TriggeredReactor<T>::SetValue(const T& value) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    m_nextValue = value;
    m_isPending = true;
  }

  template<typename T>
  template<typename E>
  void TriggeredReactor<T>::SetException(const E& exception) {
    static_assert(std::is_base_of<ReactorException, E>::value,
      "Exception must be derived from ReactorException");
    auto e = std::make_exception_ptr(exception);
    SetException(e);
  }

  template<typename T>
  void TriggeredReactor<T>::SetException(const std::exception_ptr& exception) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    m_nextValue = exception;
    m_isPending = true;
  }

  template<typename T>
  void TriggeredReactor<T>::SetComplete() {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    m_isComplete = true;
  }

  template<typename T>
  void TriggeredReactor<T>::Trigger() {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    if(!(m_isPending || m_isComplete) || m_isTriggered) {
      return;
    }
    m_isTriggered = true;
    this->SignalEvent();
  }

  template<typename T>
  void TriggeredReactor<T>::Commit() {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    if(!m_isExecuted) {
      return;
    }
    if(m_isPending) {
      m_value = std::move(*m_nextValue);
      m_isPending = false;
      this->IncrementSequenceNumber();
    }
    if(m_isComplete) {
      m_isComplete = false;
      BaseReactor::SetComplete();
    }
    m_isTriggered = false;
    m_isExecuted = false;
  }

  template<typename T>
  void TriggeredReactor<T>::Execute() {
    if(m_isExecuted) {
      return;
    }
    m_isExecuted = true;
    this->SignalUpdate();
  }

  template<typename T>
  typename TriggeredReactor<T>::Type TriggeredReactor<T>::Eval() const {
    return m_value.Get();
  }
}
}

#endif
