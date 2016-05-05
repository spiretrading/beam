#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Network/SocketThreadPool.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/DateTime.hpp"
#include "Beam/Python/MySql.hpp"
#include "Beam/Python/Network.hpp"
#include "Beam/Python/Queries.hpp"
#include "Beam/Python/Queues.hpp"
#include "Beam/Python/ServiceLocator.hpp"
#include "Beam/Python/Threading.hpp"
#include "Beam/Python/TimeService.hpp"
#include "Beam/Python/UidService.hpp"
#include "Beam/Python/Yaml.hpp"
#include "Beam/Routines/Scheduler.hpp"
#include "Beam/Threading/TimerThreadPool.hpp"

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
  ExportPtime();
  ExportTimeDuration();
  ExportMySql();
  ExportNetwork();
  ExportQueries();
  ExportQueues();
  ExportServiceLocator();
  ExportThreading();
  ExportTimeService();
  ExportUidService();
  ExportYaml();
}
