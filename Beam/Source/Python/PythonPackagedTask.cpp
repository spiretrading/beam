#include "Beam/Python/PythonPackagedTask.hpp"
#include <boost/python/extract.hpp>
#include <boost/python/make_function.hpp>
#include <boost/python/scope.hpp>
#include "Beam/Python/GilLock.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Tasks;
using namespace boost;
using namespace boost::python;
using namespace std;

namespace {
  void DoNothing() {}
}

PythonPackagedTask::PythonPackagedTask(const boost::python::object& package,
    const boost::python::tuple& args, const boost::python::dict& kw)
    : m_context{{package, args, kw}} {}

PythonPackagedTask::~PythonPackagedTask() {
  GilLock gil;
  boost::lock_guard<GilLock> lock{gil};
  m_context.reset();
}

void PythonPackagedTask::OnExecute() {
  GilLock gil;
  boost::lock_guard<GilLock> lock{gil};
  try {
    boost::python::object callable{m_context->m_package.attr("execute")};
    PyObject_Call(callable.ptr(), m_context->m_args.ptr(),
      m_context->m_kw.ptr());
  } catch(const boost::python::error_already_set&) {
    PrintError();
    SetTerminal(Task::State::FAILED);
    return;
  }
  SetTerminal();
}

void PythonPackagedTask::OnCancel() {
  GilLock gil;
  boost::lock_guard<GilLock> lock{gil};
  try {
    m_context->m_package.attr("cancel")();
  } catch(const boost::python::error_already_set&) {
    PrintError();
    SetTerminal(Task::State::FAILED);
    return;
  }
  SetTerminal();
}

PythonPackagedTaskFactory::PythonPackagedTaskFactory(
    const boost::python::object& package, const boost::python::tuple& args,
    const boost::python::dict& kw)
    : m_package{package} {
  for(int i = 0; i < len(args); ++i) {
    m_parameterNames.push_back(boost::python::extract<string>(args[i]));
    DefineProperty(m_parameterNames.back(), boost::python::object{});
  }
}

PythonPackagedTaskFactory::~PythonPackagedTaskFactory() {
  GilLock gil;
  boost::lock_guard<GilLock> lock{gil};
  m_package.reset();
}

const string& PythonPackagedTaskFactory::GetParameterName(int i) const {
  return m_parameterNames[i];
}

std::shared_ptr<Task> PythonPackagedTaskFactory::Create() {
  GilLock gil;
  boost::lock_guard<GilLock> lock{gil};
  auto t = PyTuple_New(static_cast<Py_ssize_t>(m_parameterNames.size()));
  for(std::size_t i = 0; i < m_parameterNames.size(); ++i) {
    PyTuple_SET_ITEM(t, i, boost::python::incref(
      Get<boost::python::object>(m_parameterNames[i]).ptr()));
  }
  boost::python::object parameters{boost::python::handle<>(
    boost::python::borrowed(t))};
  return std::make_shared<PythonPackagedTask>(*m_package,
    boost::python::tuple{parameters}, boost::python::dict{});
}

void PythonPackagedTaskFactory::PrepareContinuation(const Task& task) {
  auto& packagedTask = static_cast<const PythonPackagedTask&>(task);
  *m_package = packagedTask.m_context->m_package;
}

boost::python::object Beam::Tasks::MakePythonFunctionTaskFactory(
    const boost::python::tuple& args, const boost::python::dict& kw) {
  boost::python::object callable = args[0];
  boost::python::tuple a = boost::python::tuple(
    args.slice(1, boost::python::_));
  PythonFunctionPackage package{callable, make_function(&DoNothing)};
  return boost::python::object{PythonPackagedTaskFactory{
    boost::python::object{package}, a, kw}};
}
