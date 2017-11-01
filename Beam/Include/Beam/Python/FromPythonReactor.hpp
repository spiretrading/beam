#ifndef BEAM_FROM_PYTHON_REACTOR_HPP
#define BEAM_FROM_PYTHON_REACTOR_HPP
#include "Beam/Python/GilLock.hpp"
#include "Beam/Reactors/Reactor.hpp"

namespace Beam {
namespace Reactors {

  /*! \class FromPythonReactor
      \brief Wraps a Reactor of Python objects to a Reactor of type T.
      \tparam T The type to evaluate to.
   */
  template<typename T>
  class FromPythonReactor : public Reactor<T> {
    public:
      using Type = typename Reactor<T>::Type;

      //! Constructs a FromPythonReactor.
      /*!
        \param reactor The Python Reactor to wrap.
      */
      FromPythonReactor(std::shared_ptr<
        Reactor<boost::python::object>> reactor);

      virtual ~FromPythonReactor() override final;

      //! Returns the wrapped Reactor.
      const std::shared_ptr<Reactor<boost::python::object>>& GetReactor() const;

      virtual bool IsComplete() const override final;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      std::shared_ptr<Reactor<boost::python::object>> m_reactor;
  };

  //! Makes a FromPythonReactor.
  /*!
    \param reactor The Python Reactor to wrap.
  */
  template<typename T>
  auto MakeFromPythonReactor(std::shared_ptr<
      Reactor<boost::python::object>> reactor) {
    return std::make_shared<FromPythonReactor<T>>(std::move(reactor));
  }

  template<typename T>
  FromPythonReactor<T>::FromPythonReactor(
      std::shared_ptr<Reactor<boost::python::object>> reactor)
      : m_reactor{std::move(reactor)} {}

  template<typename T>
  FromPythonReactor<T>::~FromPythonReactor() {
    GilLock gil;
    boost::lock_guard<GilLock> lock{gil};
    m_reactor.reset();
  }

  template<typename T>
  const std::shared_ptr<Reactor<boost::python::object>>&
      FromPythonReactor<T>::GetReactor() const {
    return m_reactor;
  }

  template<typename T>
  bool FromPythonReactor<T>::IsComplete() const {
    GilLock gil;
    boost::lock_guard<GilLock> lock{gil};
    return m_reactor->IsComplete();
  }

  template<typename T>
  BaseReactor::Update FromPythonReactor<T>::Commit(int sequenceNumber) {
    GilLock gil;
    boost::lock_guard<GilLock> lock{gil};
    return m_reactor->Commit(sequenceNumber);
  }

  template<typename T>
  typename FromPythonReactor<T>::Type FromPythonReactor<T>::Eval() const {
    GilLock gil;
    boost::lock_guard<GilLock> lock{gil};
    return boost::python::extract<Type>{m_reactor->Eval()}();
  }
}
}

#endif
