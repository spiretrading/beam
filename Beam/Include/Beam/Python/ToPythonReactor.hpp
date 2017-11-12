#ifndef BEAM_TO_PYTHON_REACTOR_HPP
#define BEAM_TO_PYTHON_REACTOR_HPP
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Reactors/Reactor.hpp"

namespace Beam {
namespace Reactors {

  /*! \class ToPythonReactor
      \brief Wraps a Reactor of type T to a Reactor of Python objects.
      \tparam T The type to wrap.
   */
  template<typename T>
  class ToPythonReactor : public Reactor<boost::python::object> {
    public:
      using Type = typename Reactor<boost::python::object>::Type;

      //! Constructs a ToPythonReactor.
      /*!
        \param reactor The Python Reactor to wrap.
      */
      ToPythonReactor(std::shared_ptr<Reactor<T>> reactor);

      virtual ~ToPythonReactor() override final;

      //! Returns the wrapped Reactor.
      const std::shared_ptr<Reactor<T>>& GetReactor() const;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      std::shared_ptr<Reactor<T>> m_reactor;
  };

  //! Makes a ToPythonReactor.
  /*!
    \param reactor The Reactor to wrap.
  */
  template<typename T>
  auto MakeToPythonReactor(std::shared_ptr<Reactor<T>> reactor) {
    return std::make_shared<ToPythonReactor<T>>(std::move(reactor));
  }

  template<typename T>
  ToPythonReactor<T>::ToPythonReactor(std::shared_ptr<Reactor<T>> reactor)
      : m_reactor{std::move(reactor)} {}

  template<typename T>
  ToPythonReactor<T>::~ToPythonReactor() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_reactor.reset();
  }

  template<typename T>
  const std::shared_ptr<Reactor<T>>& ToPythonReactor<T>::GetReactor() const {
    return m_reactor;
  }

  template<typename T>
  BaseReactor::Update ToPythonReactor<T>::Commit(int sequenceNumber) {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_reactor->Commit(sequenceNumber);
  }

  template<typename T>
  typename ToPythonReactor<T>::Type ToPythonReactor<T>::Eval() const {
    return boost::python::object{m_reactor->Eval()};
  }
}
}

#endif
