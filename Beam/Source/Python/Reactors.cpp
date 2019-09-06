#include "Beam/Python/Reactors.hpp"
#include <Aspen/Conversions.hpp>
#include <Aspen/Python/Box.hpp>
#include <Aspen/Python/Reactor.hpp>
#include <pybind11/stl.h>
#include "Beam/Python/Beam.hpp"
#include "Beam/Reactors/AlarmReactor.hpp"
#include "Beam/Reactors/CurrentTimeReactor.hpp"
#include "Beam/Reactors/TimerReactor.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/TimeService/LocalTimeClient.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Reactors;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace pybind11;

namespace {
  auto DefaultTimerFactory(time_duration duration) {
    return std::make_unique<LiveTimer>(duration, Ref(*GetTimerThreadPool()));
  }
}

void Beam::Python::ExportAlarmReactor(pybind11::module& module) {
  module.def("alarm",
    [] (Box<ptime> expiry) {
      return to_object(AlarmReactor(LocalTimeClient(), DefaultTimerFactory,
        std::move(expiry)));
    });
}

void Beam::Python::ExportCurrentTimeReactor(pybind11::module& module) {
  module.def("current_time",
    [] {
      return to_object(CurrentTimeReactor(LocalTimeClient()));
    });
  module.def("current_time",
    [] (Box<void> pulse) {
      return to_object(CurrentTimeReactor(LocalTimeClient(), std::move(pulse)));
    });
}

void Beam::Python::ExportQueueReactor(pybind11::module& module) {
  ExportQueueReactor<object>(module, "");
}

void Beam::Python::ExportTimerReactor(pybind11::module& module) {
  module.def("timer",
    [] (Box<time_duration> period) {
      return to_object(TimerReactor<std::int64_t>(DefaultTimerFactory,
        std::move(period)));
    });
}

void Beam::Python::ExportReactors(pybind11::module& module) {
  auto submodule = module.def_submodule("reactors");
  export_box<std::int64_t>(pybind11::module::import("aspen"), "Int64");
  export_box<time_duration>(module, "TimeDuration");
  export_box<ptime>(module, "PosixTime");
  ExportAlarmReactor(submodule);
  ExportCurrentTimeReactor(submodule);
  ExportQueueReactor(submodule);
  ExportTimerReactor(submodule);
}
