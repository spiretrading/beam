#include "Beam/Python/PythonFunctionReactor.hpp"
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/Reactors.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Reactors;
using namespace boost;
using namespace boost::python;

PythonFunctionReactor::Child::Child(
    std::shared_ptr<Reactor<boost::python::object>> reactor)
    : m_reactor{std::move(reactor)},
      m_isInitialized{false} {}

PythonFunctionReactor::PythonFunctionReactor(const object& callable,
    const boost::python::tuple& args, const boost::python::dict& kw)
    : m_hasValue{false},
      m_state{BaseReactor::Update::NONE},
      m_currentSequenceNumber{-1} {
  m_context.emplace();
  m_context->m_callable = callable;
  for(int i = 0; i < boost::python::len(args); ++i) {
    boost::python::object child = args[i];
    auto reactor = ExtractReactor(child);
    m_context->m_children.push_back(reactor);
  }
  m_context->m_kw = kw;
  m_context->m_value = std::make_exception_ptr(ReactorUnavailableException{});
}

PythonFunctionReactor::~PythonFunctionReactor() {
  GilLock gil;
  boost::lock_guard<GilLock> lock{gil};
  m_context.reset();
}

bool PythonFunctionReactor::IsComplete() const {
  return m_state == BaseReactor::Update::COMPLETE;
}

BaseReactor::Update PythonFunctionReactor::Commit(int sequenceNumber) {
  if(sequenceNumber == m_currentSequenceNumber) {
    return m_update;
  } else if(sequenceNumber == 0 && m_currentSequenceNumber != -1) {
    if(m_hasValue) {
      return BaseReactor::Update::EVAL;
    }
    return BaseReactor::Update::COMPLETE;
  }
  if(m_state == BaseReactor::Update::COMPLETE) {
    return BaseReactor::Update::NONE;
  }
  auto update =
    [&] {
      if(m_state == BaseReactor::Update::NONE) {
        auto update = BaseReactor::Update::EVAL;
        for(auto& child : m_context->m_children) {
          auto childUpdate = child.m_reactor->Commit(sequenceNumber);
          if(!child.m_isInitialized) {
            if(childUpdate == BaseReactor::Update::EVAL) {
              child.m_isInitialized = true;
            } else if(childUpdate == BaseReactor::Update::COMPLETE) {
              update = BaseReactor::Update::COMPLETE;
            } else if(update != BaseReactor::Update::COMPLETE) {
              update = BaseReactor::Update::NONE;
            }
          }
        }
        if(update == BaseReactor::Update::EVAL) {
          m_state = BaseReactor::Update::EVAL;
        }
        return update;
      }
      auto update = BaseReactor::Update::NONE;
      auto hasCompletion = false;
      for(auto& child : m_context->m_children) {
        auto childUpdate = child.m_reactor->Commit(sequenceNumber);
        if(childUpdate == BaseReactor::Update::EVAL) {
          update = BaseReactor::Update::EVAL;
        } else if(childUpdate == BaseReactor::Update::COMPLETE) {
          hasCompletion = true;
        }
      }
      if(update == BaseReactor::Update::NONE && hasCompletion) {
        if(AreParametersComplete()) {
          update = BaseReactor::Update::COMPLETE;
        }
      }
      return update;
    }();
  m_update = update;
  if(m_update == BaseReactor::Update::EVAL) {
    if(AreParametersComplete()) {
      m_state = BaseReactor::Update::COMPLETE;
    }
    if(!UpdateEval()) {
      if(m_state == BaseReactor::Update::COMPLETE) {
        m_update = BaseReactor::Update::COMPLETE;
      } else {
        m_update = BaseReactor::Update::NONE;
      }
    } else {
      m_hasValue = true;
    }
  } else if(m_update == BaseReactor::Update::COMPLETE) {
    m_state = BaseReactor::Update::COMPLETE;
  }
  m_currentSequenceNumber = sequenceNumber;
  return m_update;
}

PythonFunctionReactor::Type PythonFunctionReactor::Eval() const {
  return m_context->m_value.Get();
}

bool PythonFunctionReactor::AreParametersComplete() const {
  for(auto& child : m_context->m_children) {
    if(!child.m_reactor->IsComplete()) {
      return false;
    }
  }
  return true;
}

bool PythonFunctionReactor::UpdateEval() {
  std::vector<boost::python::object> parameters;
  for(auto& child : m_context->m_children) {
    parameters.push_back(boost::python::object{Try(
      [&] {
        return child.m_reactor->Eval();
      })});
  }
  auto t = PyTuple_New(static_cast<Py_ssize_t>(parameters.size()));
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    PyTuple_SET_ITEM(t, i, parameters[i].ptr());
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
