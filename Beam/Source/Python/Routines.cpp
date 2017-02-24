#include "Beam/Python/Routines.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/GilRelease.hpp"
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
}

void Beam::Python::ExportRoutineHandler() {
  class_<RoutineHandler, noncopyable>("RoutineHandler", init<>())
    .def(init<Routine::Id>())
    .add_property("id", &RoutineHandler::GetId)
    .def("detach", &RoutineHandler::Detach)
    .def("wait", BlockingFunction(&RoutineHandler::Wait));
  def("flush_pending_routines", BlockingFunction(&FlushPendingRoutines));
}

void Beam::Python::ExportRoutineHandlerGroup() {
  class_<RoutineHandlerGroup, noncopyable>("RoutineHandlerGroup", init<>())
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
  ExportRoutineHandler();
  ExportRoutineHandlerGroup();
}
