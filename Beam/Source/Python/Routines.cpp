#include "Beam/Python/Routines.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Routines;
using namespace boost;
using namespace boost::python;
using namespace std;

void Beam::Python::ExportRoutines() {
  string nestedName = extract<string>(scope().attr("__name__") + ".routines");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("routines") = nestedModule;
  scope parent = nestedModule;
  def("flush_pending_routines", BlockingFunction(&FlushPendingRoutines));
}
