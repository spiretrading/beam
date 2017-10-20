#ifndef BEAM_PYTHON_PACKAGED_TASK_HPP
#define BEAM_PYTHON_PACKAGED_TASK_HPP
#include <boost/python/dict.hpp>
#include <boost/python/object.hpp>
#include <boost/python/tuple.hpp>
#include "Beam/Tasks/BasicTask.hpp"
#include "Beam/Tasks/Tasks.hpp"

namespace Beam {
namespace Tasks {

  /*! \class PythonPackagedTask
      \brief Implements a PackagedTask for use within Python.
   */
  class PythonPackagedTask : public BasicTask {
    public:

      //! Constructs a PythonPackagedTask.
      /*!
        \param package The package used to execute the Task.
        \param args The arguments to the function.
        \param kw The keyword arguments to the function.
      */
      PythonPackagedTask(const boost::python::object& package,
        const boost::python::tuple& args, const boost::python::dict& kw);

    protected:
      virtual void OnExecute() override final;

      virtual void OnCancel() override final;

    private:
      friend class PythonPackagedTaskFactory;
      boost::python::object m_package;
      boost::python::object m_args;
      boost::python::object m_kw;
  };

  /*! \class PythonPackagedTaskFactory
      \brief Implements the TaskFactory for PythonPackagedTasks.
   */
  class PythonPackagedTaskFactory :
      public BasicTaskFactory<PythonPackagedTaskFactory> {
    public:

      //! Constructs a PythonPackagedTaskFactory.
      /*!
        \param package The package used to execute the Task.
        \param args The names of the parameters.
        \param kw The keyword arguments (ignored).
      */
      PythonPackagedTaskFactory(const boost::python::object& package,
        const boost::python::tuple& args, const boost::python::dict& kw);

      //! Returns the name of a parameter.
      /*!
        \param i The index of the parameter.
        \return The name of the parameter at index <i>i</i>.
      */
      const std::string& GetParameterName(int i) const;

      virtual std::shared_ptr<Task> Create() override final;

      virtual void PrepareContinuation(const Task& task) override final;

    private:
      boost::python::object m_package;
      std::vector<std::string> m_parameterNames;
  };
}
}

#endif
