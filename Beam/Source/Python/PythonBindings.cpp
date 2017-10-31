#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Network/SocketThreadPool.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/DateTime.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/Expect.hpp"
#include "Beam/Python/Function.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/IO.hpp"
#include "Beam/Python/MySql.hpp"
#include "Beam/Python/Network.hpp"
#include "Beam/Python/Optional.hpp"
#include "Beam/Python/Queries.hpp"
#include "Beam/Python/Queues.hpp"
#include "Beam/Python/Rational.hpp"
#include "Beam/Python/Reactors.hpp"
#include "Beam/Python/Routines.hpp"
#include "Beam/Python/ServiceLocator.hpp"
#include "Beam/Python/SharedObject.hpp"
#include "Beam/Python/SignalsSlots.hpp"
#include "Beam/Python/Tasks.hpp"
#include "Beam/Python/Threading.hpp"
#include "Beam/Python/TimeService.hpp"
#include "Beam/Python/UidService.hpp"
#include "Beam/Python/WebServices.hpp"
#include "Beam/Python/Yaml.hpp"
#include "Beam/Routines/Scheduler.hpp"
#include "Beam/Threading/TimerThreadPool.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::Routines;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::python;

SocketThreadPool* Beam::Python::GetSocketThreadPool() {
  static auto pool = new SocketThreadPool{
    boost::thread::hardware_concurrency()};
  return pool;
}

TimerThreadPool* Beam::Python::GetTimerThreadPool() {
  static auto pool = new TimerThreadPool{boost::thread::hardware_concurrency()};
  return pool;
}

BOOST_PYTHON_MODULE(beam) {
  PyEval_InitThreads();
  ExportRuntimeError();
  def("is_running", &IsRunning);
  def("received_kill_event", &ReceivedKillEvent);
  def("wait_for_kill_event", BlockingFunction(&WaitForKillEvent));
  python_optional<std::string>();
  ExportPtime();
  ExportTimeDuration();
  ExportExpect<object>("Expect");
  ExportFunctions();
  ExportIO();
  ExportMySql();
  ExportNetwork();
  ExportQueries();
  ExportQueues();
  ExportReactors();
  ExportRoutines();
  ExportServiceLocator();
  ExportSharedObject();
  ExportSignalsSlots();
  ExportTasks();
  ExportThreading();
  ExportTimeService();
  ExportUidService();
  ExportWebServices();
  ExportYaml();
  ExportRational<boost::rational<int>>();
}
