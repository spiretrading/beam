#include "Beam/Python/PythonPackagedTask.hpp"
#include <boost/python/extract.hpp>
#include "Beam/Python/GilLock.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Tasks;
using namespace boost;
using namespace boost::python;
using namespace std;

PythonPackagedTask::PythonPackagedTask(const boost::python::object& package,
    const boost::python::tuple& args, const boost::python::dict& kw)
    : m_package{package},
      m_args{args},
      m_kw{kw} {}

void PythonPackagedTask::OnExecute() {
  GilLock gil;
  boost::lock_guard<GilLock> lock{gil};
  try {
    m_package["execute"](m_args, m_kw);
  } catch(const boost::python::error_already_set&) {
    PrintError();
    SetTerminal(Task::State::FAILED);
    return;
  }
  SetTerminal();
}

void PythonPackagedTask::OnCancel() {
  try {
    m_package["cancel"]();
  } catch(const boost::python::error_already_set&) {
    PrintError();
    SetTerminal(Task::State::FAILED);
    return;
  }
  SetTerminal();
}

PythonPackagedTaskFactory::PythonPackagedTaskFactory(
    const boost::python::object& package,
    const boost::python::tuple& args, const boost::python::dict& kw)
    : m_package{package} {
  for(int i = 0; i < len(args); ++i) {
    m_parameterNames.push_back(boost::python::extract<string>(args[i]));
  }
}

const string& PythonPackagedTaskFactory::GetParameterName(int i) const {
  return m_parameterNames[i];
}

std::shared_ptr<Task> PythonPackagedTaskFactory::Create() {
  return nullptr;
}

void PythonPackagedTaskFactory::PrepareContinuation(const Task& task) {
}
