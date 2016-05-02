#include "Beam/Python/Threading.hpp"
#include <functional>
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/Queues.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace std;

namespace {
  LiveTimer* MakeLiveTimer(time_duration interval) {
    return new LiveTimer(interval, Ref(*GetTimerThreadPool()));
  }
}

void Beam::Python::ExportLiveTimer() {
  class_<LiveTimer, boost::noncopyable>("LiveTimer", no_init)
    .def("__init__", make_constructor(&MakeLiveTimer))
    .def("start", BlockingFunction(&LiveTimer::Start))
    .def("cancel", BlockingFunction(&LiveTimer::Cancel))
    .def("wait", BlockingFunction(&LiveTimer::Wait))
    .def("get_publisher", &LiveTimer::GetPublisher,
      return_value_policy<reference_existing_object>());
  ExportPublisher<Timer::Result>("TimerResultPublisher");
}

void Beam::Python::ExportThreading() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".threading");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("threading") = nestedModule;
  scope parent = nestedModule;
  ExportLiveTimer();
  ExportTimer();
  ExportTriggerTimer();
}

void Beam::Python::ExportTimer() {
  {
    scope outer = class_<Timer, boost::noncopyable>("Timer", no_init);
    enum_<Timer::Result::Type>("Result")
      .value("NONE", Timer::Result::NONE)
      .value("EXPIRED", Timer::Result::EXPIRED)
      .value("CANCELED", Timer::Result::CANCELED)
      .value("FAIL", Timer::Result::FAIL);
  }
  ExportEnum<Timer::Result>();
}

void Beam::Python::ExportTriggerTimer() {
  class_<TriggerTimer, boost::noncopyable>("TriggerTimer", init<>())
    .def("trigger", BlockingFunction(&TriggerTimer::Trigger))
    .def("start", BlockingFunction(&TriggerTimer::Start))
    .def("cancel", BlockingFunction(&TriggerTimer::Cancel))
    .def("wait", BlockingFunction(&TriggerTimer::Wait))
    .def("get_publisher", &TriggerTimer::GetPublisher,
      return_value_policy<reference_existing_object>());
}
