#ifndef BEAM_PYTHON_FUNCTION_REACTOR_HPP
#define BEAM_PYTHON_FUNCTION_REACTOR_HPP
#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/object.hpp>
#include <boost/python/tuple.hpp>
#include "Beam/Python/Python.hpp"
#include "Beam/Reactors/CommitReactor.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {

  /*! \class PythonFunctionReactor
      \brief Implements the Python version of the FunctionReactor.
   */
  class PythonFunctionReactor : public Reactor<boost::python::object> {
    public:
      using Type = Reactor<boost::python::object>::Type;

      //! Constructs a PythonFunctionReactor.
      /*!
        \param callable The function to call.
        \param args The Reactors to use as parameters to the <i>callable</i>.
        \param kw Unused.
      */
      PythonFunctionReactor(const boost::python::object& callable,
        const boost::python::tuple& args, const boost::python::dict& kw);

      virtual ~PythonFunctionReactor() override final;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      struct Context {
        boost::python::object m_callable;
        std::vector<std::shared_ptr<Reactor<boost::python::object>>> m_children;
        boost::python::dict m_kw;
        Expect<Type> m_value;
      };
      boost::optional<Context> m_context;
      boost::optional<CommitReactor> m_commitReactor;
      int m_currentSequenceNumber;
      BaseReactor::Update m_update;
      BaseReactor::Update m_state;

      bool UpdateEval();
  };
}
}

#endif
