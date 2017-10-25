#ifndef BEAM_PYTHON_FUNCTION_REACTOR_HPP
#define BEAM_PYTHON_FUNCTION_REACTOR_HPP
#include <boost/optional/optional.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/object.hpp>
#include <boost/python/tuple.hpp>
#include "Beam/Python/Python.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {
  class PythonFunctionReactor : public Reactor<boost::python::object> {
    public:
      using Type = Reactor<boost::python::object>::Type;

      PythonFunctionReactor(const boost::python::object& callable,
        const boost::python::tuple& args, const boost::python::dict& kw);

      virtual ~PythonFunctionReactor() override final;

      virtual bool IsComplete() const override final;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      struct Context {
        boost::python::object m_callable;
        boost::python::tuple m_args;
        boost::python::dict m_kw;
      };
      boost::optional<Context> m_context;
      Expect<Type> m_value;
      bool m_hasValue;
      BaseReactor::Update m_state;
      BaseReactor::Update m_update;
      int m_initializationCount;
      int m_currentSequenceNumber;

      bool AreParametersComplete() const;
      BaseReactor::Update UpdateEval();
  };
}
}

#endif
