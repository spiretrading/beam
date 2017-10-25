#include "Beam/Python/PythonFunctionReactor.hpp"
#include <boost/python/extract.hpp>
#include "Beam/Python/GilLock.hpp"
#include "Beam/Utilities/Expect.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Reactors;
using namespace boost;
using namespace boost::python;

PythonFunctionReactor::PythonFunctionReactor(const object& callable,
    const boost::python::tuple& args, const boost::python::dict& kw)
  : m_context{{callable, args, kw}},
    m_value{std::make_exception_ptr(ReactorUnavailableException{})},
    m_hasValue{false},
    m_state{BaseReactor::Update::NONE},
    m_initializationCount{0},
    m_currentSequenceNumber{-1} {}

PythonFunctionReactor::~PythonFunctionReactor() {
  GilLock gil;
  boost::lock_guard<GilLock> lock{gil};
  m_context.reset();
}

bool PythonFunctionReactor::IsComplete() const {
  return m_state == BaseReactor::Update::COMPLETE;
}

BaseReactor::Update PythonFunctionReactor::Commit(int sequenceNumber) {
  GilLock gil;
  boost::lock_guard<GilLock> lock{gil};
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
      if(boost::python::len(m_context->m_args) == 0) {
        if(sequenceNumber == 0) {
          return BaseReactor::Update::EVAL;
        } else {
          return BaseReactor::Update::NONE;
        }
      }
      if(m_initializationCount != boost::python::len(m_context->m_args)) {
        m_initializationCount = 0;
        for(int i = 0; i < boost::python::len(m_context->m_args); ++i) {
          boost::python::object child = m_context->m_args[i];
          std::shared_ptr<BaseReactor> reactor =
            boost::python::extract<std::shared_ptr<BaseReactor>>(child);
          if(reactor->Commit(0) != BaseReactor::Update::NONE ||
              reactor->Commit(sequenceNumber) != BaseReactor::Update::NONE) {
            ++m_initializationCount;
          }
        }
        if(m_initializationCount != boost::python::len(m_context->m_args)) {
          m_currentSequenceNumber = sequenceNumber;
          m_update = BaseReactor::Update::NONE;
          return BaseReactor::Update::NONE;
        }
      }
      auto commit = BaseReactor::Update::NONE;
      for(int i = 0; i < boost::python::len(m_context->m_args); ++i) {
        boost::python::object child = m_context->m_args[i];
        std::shared_ptr<BaseReactor> reactor =
          boost::python::extract<std::shared_ptr<BaseReactor>>(child);
        auto reactorUpdate = reactor->Commit(sequenceNumber);
        if(reactorUpdate == BaseReactor::Update::COMPLETE) {
          if(commit == BaseReactor::Update::NONE ||
              commit == BaseReactor::Update::COMPLETE) {
            commit = reactorUpdate;
          }
        } else {
          commit = reactorUpdate;
        }
      }
      return commit;
    }();
  if(update == BaseReactor::Update::NONE) {
    return update;
  }
  m_update = update;
  if(m_update == BaseReactor::Update::EVAL) {
    auto hasEval = UpdateEval();
    if(AreParametersComplete()) {
      m_state = BaseReactor::Update::COMPLETE;
    }
    if(hasEval == BaseReactor::Update::NONE) {
      if(boost::python::len(m_context->m_args) == 0) {
        m_update = BaseReactor::Update::COMPLETE;
      } else {
        m_update = BaseReactor::Update::NONE;
      }
    } else if(hasEval == BaseReactor::Update::EVAL) {
      m_hasValue = true;
    } else {
      m_hasValue = true;
      m_state = BaseReactor::Update::COMPLETE;
    }
  } else if(m_update == BaseReactor::Update::COMPLETE) {
    if(AreParametersComplete()) {
      m_state = BaseReactor::Update::COMPLETE;
    } else {
      m_update = BaseReactor::Update::NONE;
    }
  }
  m_currentSequenceNumber = sequenceNumber;
  return m_update;
}

PythonFunctionReactor::Type PythonFunctionReactor::Eval() const {
  return m_value.Get();
}

bool PythonFunctionReactor::AreParametersComplete() const {
  for(int i = 0; i < boost::python::len(m_context->m_args); ++i) {
    boost::python::object child = m_context->m_args[i];
    std::shared_ptr<BaseReactor> reactor =
      boost::python::extract<std::shared_ptr<BaseReactor>>(child);
    if(!reactor->IsComplete()) {
      return false;
    }
  }
  return true;
}

BaseReactor::Update PythonFunctionReactor::UpdateEval() {
  std::vector<boost::python::object> parameters;
  for(int i = 0; i < boost::python::len(m_context->m_args); ++i) {
    boost::python::object child = m_context->m_args[i];
    std::shared_ptr<Reactor<object>> reactor =
      boost::python::extract<std::shared_ptr<Reactor<object>>>(child);
    parameters.push_back(Try(
      [&] {
        return reactor->Eval();
      }));
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
  } catch(const std::exception&) {
    m_value = std::current_exception();
    return BaseReactor::Update::EVAL;
  }
}
