#ifndef BEAM_PYTHON_WRAPPER_REACTOR_HPP
#define BEAM_PYTHON_WRAPPER_REACTOR_HPP
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/Python.hpp"
#include "Beam/Reactors/Reactor.hpp"

namespace Beam {
namespace Python {

  /*! \class PythonWrapperReactor
      \brief Wraps a Python Reactor in a way that allows it to safely run
             asynchronously.
   */
  class PythonWrapperReactor : public Reactors::Reactor<boost::python::object> {
    public:

      //! Constructs a PythonWrapperReactor.
      /*!
        \param reactor The Reactor to wrap.
      */
      PythonWrapperReactor(std::shared_ptr<Reactors::Reactor<Type>> reactor);

      virtual bool IsComplete() const override final;

      virtual Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      std::shared_ptr<Reactors::Reactor<Type>> m_reactor;
  };

  //! Makes a PythonWrapperReactor.
  /*!
    \param reactor The Reactor to wrap.
  */
  inline std::shared_ptr<Reactors::Reactor<boost::python::object>>
      MakePythonWrapperReactor(
      std::shared_ptr<Reactors::Reactor<boost::python::object>> reactor) {
    return std::make_shared<PythonWrapperReactor>(std::move(reactor));
  }

  inline PythonWrapperReactor::PythonWrapperReactor(
      std::shared_ptr<Reactors::Reactor<Type>> reactor)
      : m_reactor{std::move(reactor)} {}

  inline bool PythonWrapperReactor::IsComplete() const {
    GilLock gil;
    boost::lock_guard<GilLock> lock{gil};
    return m_reactor->IsComplete();
  }

  inline Reactors::BaseReactor::Update PythonWrapperReactor::Commit(
      int sequenceNumber) {
    GilLock gil;
    boost::lock_guard<GilLock> lock{gil};
    return m_reactor->Commit(sequenceNumber);
  }

  inline PythonWrapperReactor::Type PythonWrapperReactor::Eval() const {
    return m_reactor->Eval();
  }
}
}

#endif
