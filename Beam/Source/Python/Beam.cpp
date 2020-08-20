#include "Beam/Python/Beam.hpp"
#include <datetime.h>
#include <pybind11/pybind11.h>
#include "Beam/Network/SocketThreadPool.hpp"
#include "Beam/Threading/TimerThreadPool.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::Threading;
using namespace pybind11;

template struct Beam::Routines::Details::CurrentRoutineGlobal<void>;
template struct Beam::Routines::Details::NextId<void>;

SocketThreadPool* Beam::Python::GetSocketThreadPool() {
  static auto pool = new SocketThreadPool(
    boost::thread::hardware_concurrency());
  return pool;
}

TimerThreadPool* Beam::Python::GetTimerThreadPool() {
  static auto pool = new TimerThreadPool(boost::thread::hardware_concurrency());
  return pool;
}

PYBIND11_MODULE(_beam, module) {
  ExportIO(module);
  ExportKeyValuePair(module);
  ExportNetwork(module);
  ExportQueries(module);
  ExportQueues(module);
  ExportReactors(module);
  ExportRoutines(module);
  ExportServiceLocator(module);
  ExportSql(module);
  ExportThreading(module);
  ExportTimeService(module);
  ExportUidService(module);
  ExportWebServices(module);
  ExportYaml(module);
}
