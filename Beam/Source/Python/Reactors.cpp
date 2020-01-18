#include "Beam/Python/Reactors.hpp"
#include <Aspen/Conversions.hpp>
#include <Aspen/Python/Box.hpp>
#include <Aspen/Python/Reactor.hpp>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include "Beam/Python/Beam.hpp"
#include "Beam/Reactors/QueueReactor.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Reactors/AlarmReactor.hpp"
#include "Beam/Reactors/CurrentTimeReactor.hpp"
#include "Beam/Reactors/TimerReactor.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/TimeService/LocalTimeClient.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Queries;
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
    [] (SharedBox<ptime> expiry) {
      return to_object(AlarmReactor(LocalTimeClient(), DefaultTimerFactory,
        std::move(expiry)));
    });
  module.def("alarm",
    [] (VirtualTimeClient& timeClient,
        std::function<std::shared_ptr<VirtualTimer> (time_duration)>
        timerFactory, SharedBox<ptime> expiry) {
      return to_object(AlarmReactor(&timeClient, std::move(timerFactory),
        std::move(expiry)));
    });
}

void Beam::Python::ExportCurrentTimeReactor(pybind11::module& module) {
  module.def("current_time",
    [] {
      return to_object(CurrentTimeReactor(LocalTimeClient()));
    });
  module.def("current_time",
    [] (VirtualTimeClient& timeClient) {
      return to_object(CurrentTimeReactor(&timeClient));
    });
  module.def("current_time",
    [] (SharedBox<void> pulse) {
      return to_object(CurrentTimeReactor(LocalTimeClient(), std::move(pulse)));
    });
  module.def("current_time",
    [] (VirtualTimeClient& timeClient, SharedBox<void> pulse) {
      return to_object(CurrentTimeReactor(&timeClient, std::move(pulse)));
    });
}

void Beam::Python::ExportPublisherReactor(pybind11::module& module) {
  module.def("monitor",
    [] (std::shared_ptr<Publisher<object>> publisher) {
      return shared_box(PublisherReactor(std::move(publisher)));
    });
}

void Beam::Python::ExportQueryReactor(pybind11::module& module) {
  module.def("query",
    [] (std::function<
        void (const BasicQuery<object>&, std::shared_ptr<QueueWriter<object>>)>
        submissionFunction, SharedBox<BasicQuery<object>> query) {
      return shared_box(QueryReactor<object>(std::move(submissionFunction),
        std::move(query)));
    });
}

void Beam::Python::ExportQueueReactor(pybind11::module& module) {
  export_reactor<QueueReactor<object>>(module, "QueueReactor")
    .def(pybind11::init<std::shared_ptr<QueueReader<object>>>());
}

void Beam::Python::ExportTimerReactor(pybind11::module& module) {
  module.def("timer",
    [] (SharedBox<time_duration> period) {
      return to_object(TimerReactor<std::int64_t>(DefaultTimerFactory,
        std::move(period)));
    });
  module.def("timer",
    [] (std::function<std::shared_ptr<VirtualTimer> (time_duration)>
        timerFactory, SharedBox<time_duration> period) {
      return to_object(TimerReactor<std::int64_t>(std::move(timerFactory),
        std::move(period)));
    });
}

void Beam::Python::ExportReactors(pybind11::module& module) {
  auto submodule = module.def_submodule("reactors");
  auto aspenModule = pybind11::module::import("aspen");
  export_box<std::int64_t>(aspenModule, "Int64");
  export_box<time_duration>(module, "TimeDuration");
  export_box<ptime>(module, "PosixTime");
  ExportAlarmReactor(submodule);
  ExportCurrentTimeReactor(submodule);
  ExportPublisherReactor(submodule);
  ExportQueryReactor(submodule);
  ExportQueueReactor(submodule);
  ExportTimerReactor(submodule);
}
