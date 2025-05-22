#include "Beam/Python/Beam.hpp"
#include <datetime.h>
#include <pybind11/pybind11.h>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::Threading;
using namespace pybind11;

template struct Beam::Routines::Details::CurrentRoutineGlobal<void>;
template struct Beam::Routines::Details::NextId<void>;

PYBIND11_MODULE(beam, m) {
  ExportIO(m);
  ExportCodecs(m);
  ExportJson(m);
  ExportKeyValuePair(m);
  ExportNetwork(m);
  ExportQueues(m);
  ExportQueries(m);
  ExportReactors(m);
  ExportRegistryService(m);
  ExportRoutines(m);
  ExportServiceLocator(m);
  ExportSql(m);
  ExportThreading(m);
  ExportTimeService(m);
  ExportUidService(m);
  ExportWebServices(m);
  ExportYaml(m);
  m.def("is_running", &IsRunning);
  m.def("received_kill_event", &ReceivedKillEvent);
  m.def("wait_for_kill_event", &WaitForKillEvent, call_guard<GilRelease>());
}
