#include "Beam/Python/Routines.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/GilLock.hpp"
#include "Beam/Routines/RoutineException.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Routines;
using namespace boost;
using namespace boost::python;
using namespace std;

namespace {
  void RoutineHandlerGroupAddRoutineHandler(RoutineHandlerGroup& routines,
      RoutineHandler* handler) {
    routines.Add(std::move(*handler));
  }

  void RoutineHandlerGroupSpawn(RoutineHandlerGroup& routines,
      object object) {
    routines.Spawn(
      [=] {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        object();
      });
  }

  Routine::Id PythonSpawn(object object) {
    return Spawn(
      [=] {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        object();
      });
  }

  void DeleteRoutineHandler(RoutineHandler& routine) {
    routine.Wait();
  }

  void DeleteRoutineHandlerGroup(RoutineHandlerGroup& routines) {
    routines.Wait();
  }
}

#ifdef _MSC_VER
namespace boost {
  template<> inline const volatile Eval<object>* get_pointer(
      const volatile Eval<object>* p) {
    return p;
  }
}
#endif

void Beam::Python::ExportBaseAsync() {
  {
    scope outer = class_<BaseAsync, noncopyable>("BaseAsync", no_init);
    enum_<BaseAsync::State>("State")
      .value("PENDING", BaseAsync::State::PENDING)
      .value("COMPLETE", BaseAsync::State::COMPLETE)
      .value("EXCEPTION", BaseAsync::State::EXCEPTION);
  }
}

void Beam::Python::ExportBaseEval() {
  class_<BaseEval, noncopyable>("BaseEval", no_init);
}

void Beam::Python::ExportPythonAsync() {
  ExportAsync<object>("Async");
}

void Beam::Python::ExportPythonEval() {
  ExportEval<object>("Eval");
}

void Beam::Python::ExportRoutineHandler() {
  class_<RoutineHandler, noncopyable>("RoutineHandler", init<>())
    .def(init<Routine::Id>())
    .def("__del__", BlockingFunction(&DeleteRoutineHandler))
    .add_property("id", &RoutineHandler::GetId)
    .def("detach", &RoutineHandler::Detach)
    .def("wait", BlockingFunction(&RoutineHandler::Wait));
  def("flush_pending_routines", BlockingFunction(&FlushPendingRoutines));
}

void Beam::Python::ExportRoutineHandlerGroup() {
  class_<RoutineHandlerGroup, noncopyable>("RoutineHandlerGroup", init<>())
    .def("__del__", BlockingFunction(&DeleteRoutineHandlerGroup))
    .def("add", static_cast<void (RoutineHandlerGroup::*)(Routine::Id)>(
      &RoutineHandlerGroup::Add))
    .def("add", &RoutineHandlerGroupAddRoutineHandler)
    .def("spawn", &RoutineHandlerGroupSpawn)
    .def("wait", BlockingFunction(&RoutineHandlerGroup::Wait));
}

void Beam::Python::ExportRoutines() {
  string nestedName = extract<string>(scope().attr("__name__") + ".routines");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("routines") = nestedModule;
  scope parent = nestedModule;
  ExportBaseAsync();
  ExportBaseEval();
  ExportPythonEval();
  ExportPythonAsync();
  ExportRoutineHandler();
  ExportRoutineHandlerGroup();
  ExportException<RoutineException, std::runtime_error>("RoutineException")
    .def(init<const string&>());
  def("spawn", &PythonSpawn);
  def("wait", &Wait);
}
