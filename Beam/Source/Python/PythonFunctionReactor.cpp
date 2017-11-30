#include "Beam/Python/PythonFunctionReactor.hpp"
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/Reactors.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"
#include "Beam/Utilities/Algorithm.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Reactors;
using namespace boost;
using namespace boost::python;

PythonFunctionReactor::PythonFunctionReactor(const object& callable,
    const boost::python::tuple& args, const boost::python::dict& kw)
    : m_currentSequenceNumber{-1},
      m_state{BaseReactor::Update::NONE} {
  m_context.emplace();
  m_context->m_callable = callable;
  for(int i = 0; i < boost::python::len(args); ++i) {
    boost::python::object child = args[i];
    auto reactor = ExtractReactor(child);
    m_context->m_children.push_back(reactor);
  }
  m_context->m_kw = kw;
  m_context->m_value = std::make_exception_ptr(ReactorUnavailableException{});
  std::vector<BaseReactor*> dependencies;
  for(auto& child : m_context->m_children) {
    dependencies.push_back(child.get());
  }
  m_commitReactor.emplace(std::move(dependencies));
}

PythonFunctionReactor::~PythonFunctionReactor() {
  GilLock gil;
  boost::lock_guard<GilLock> lock{gil};
  m_context.reset();
}

BaseReactor::Update PythonFunctionReactor::Commit(int sequenceNumber) {
  if(m_currentSequenceNumber == sequenceNumber) {
    return m_update;
  } else if(sequenceNumber == 0 && m_currentSequenceNumber != -1) {
    return m_state;
  } else if(IsComplete(m_state)) {
    return BaseReactor::Update::NONE;
  }
  m_update = m_commitReactor->Commit(sequenceNumber);
  if(HasEval(m_update)) {
    if(!UpdateEval()) {
      if(IsComplete(m_update)) {
        m_update = BaseReactor::Update::COMPLETE;
      } else {
        m_update = BaseReactor::Update::NONE;
      }
    }
  }
  m_currentSequenceNumber = sequenceNumber;
  Combine(m_state, m_update);
  return m_update;
}

PythonFunctionReactor::Type PythonFunctionReactor::Eval() const {
  return m_context->m_value.Get();
}

bool PythonFunctionReactor::UpdateEval() {
  std::vector<boost::python::object> parameters;
  for(auto& child : m_context->m_children) {
    parameters.push_back(boost::python::object{Try(
      [&] {
        return child->Eval();
      })});
  }
  auto t = PyTuple_New(static_cast<Py_ssize_t>(parameters.size()));
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    PyTuple_SET_ITEM(t, i, boost::python::incref(parameters[i].ptr()));
  }
  boost::python::object parameterTuple{boost::python::handle<>(
    boost::python::borrowed(t))};
  try {
    auto rawResult = PyObject_Call(m_context->m_callable.ptr(),
      parameterTuple.ptr(), m_context->m_kw.ptr());
    boost::python::object result{boost::python::handle<>(
      boost::python::borrowed(rawResult))};
    if(result == boost::python::object{}) {
      return false;
    }
    m_context->m_value = result;
  } catch(const std::exception&) {
    m_context->m_value = std::current_exception();
  }
  return true;
}
