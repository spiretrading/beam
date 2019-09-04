#include "Beam/Python/Reactors.hpp"
#include <aspen/Box.hpp>
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/SharedObject.hpp"
#include "Beam/Reactors/AlarmReactor.hpp"
#include "Beam/Reactors/CurrentTimeReactor.hpp"
#include "Beam/Reactors/PublisherReactor.hpp"
#include "Beam/Reactors/QueryReactor.hpp"
#include "Beam/Reactors/TimerReactor.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Threading/VirtualTimer.hpp"
#include "Beam/TimeService/LocalTimeClient.hpp"
#include "Beam/TimeService/VirtualTimeClient.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Reactors;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace std;

#if 0
namespace {
  auto MakePythonAlarmReactor(VirtualTimeClient* timeClient,
      const object& timerFactory,
      const boost::python::object& expiryReactor) {
    auto properTimerFactory =
      [timerFactory = SharedObject{timerFactory}]
          (const time_duration& expiry) {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        return MakeVirtualTimer(extract<std::shared_ptr<VirtualTimer>>{
          (*timerFactory)(expiry)}());
      };
    return std::static_pointer_cast<Reactor<bool>>(MakeAlarmReactor(timeClient,
      properTimerFactory, ExtractReactor<ptime>(expiryReactor)));
  }

  auto MakePythonDefaultAlarmReactor(
      const boost::python::object& expiryReactor) {
    auto timerFactory =
      [] (const time_duration& duration) {
        return std::make_unique<LiveTimer>(duration,
          Ref(*GetTimerThreadPool()));
      };
    return std::static_pointer_cast<Reactor<bool>>(MakeAlarmReactor(
      std::make_shared<LocalTimeClient>(), timerFactory,
      ExtractReactor<ptime>(expiryReactor)));
  }

  auto MakePythonCurrentTimeReactor(VirtualTimeClient* timeClient) {
    return std::static_pointer_cast<PythonReactor>(
      MakeToPythonReactor(static_pointer_cast<Reactor<ptime>>(
      MakeCurrentTimeReactor(timeClient))));
  }

  auto MakePythonDefaultCurrentTimeReactor() {
    return std::static_pointer_cast<PythonReactor>(
      MakeToPythonReactor(static_pointer_cast<Reactor<ptime>>(
      MakeCurrentTimeReactor())));
  }

  auto MakePythonPublisherReactor(const Publisher<object>& publisher) {
    return std::static_pointer_cast<PythonReactor>(
      MakePublisherReactor(publisher));
  }

  auto MakePythonQueryReactor(boost::python::object submissionFunction,
      const Queries::BasicQuery<object>& query) {
    return std::static_pointer_cast<PythonReactor>(
      MakeQueryReactor<boost::python::object>(
        [submissionFunction] (const Queries::BasicQuery<object>& query,
            std::shared_ptr<QueueWriter<object>> queue) {
          try {
            submissionFunction(query, queue);
          } catch(const boost::python::error_already_set&) {
            PrintError();
          }
        }, query));
  }

  auto MakePythonCurrentQueryReactor(boost::python::object submissionFunction,
      const object& index) {
    return std::static_pointer_cast<PythonReactor>(
      MakeCurrentQueryReactor<boost::python::object>(
        [submissionFunction] (const Queries::BasicQuery<object>& query,
            std::shared_ptr<QueueWriter<object>> queue) {
          try {
            submissionFunction(query, queue);
          } catch(const boost::python::error_already_set&) {
            PrintError();
          }
        }, index));
  }

  auto MakePythonRealTimeQueryReactor(boost::python::object submissionFunction,
      const object& index) {
    return std::static_pointer_cast<PythonReactor>(
      MakeRealTimeQueryReactor<boost::python::object>(
        [submissionFunction] (const Queries::BasicQuery<object>& query,
            std::shared_ptr<QueueWriter<object>> queue) {
          try {
            submissionFunction(query, queue);
          } catch(const boost::python::error_already_set&) {
            PrintError();
          }
        }, index));
  }

  auto MakePythonQueueReactor(std::shared_ptr<QueueReader<object>> queue) {
    return std::static_pointer_cast<PythonReactor>(MakeQueueReactor(queue));
  }
#endif
namespace {
  template<typename T>
  struct PythonReactor {
    using Type = T;

    PythonReactor(object reactor)
      : m_reactor(std::move(reactor)) {}

    Aspen::State commit(int sequence) noexcept {
      return static_cast<Aspen::State>(
        extract<int>(m_reactor.attr("commit")(sequence).attr("__int__")())());
    }

    Type eval() const {
      return extract<Type>(m_reactor.attr("eval")());
    }

    object m_reactor;
  };

  auto MakePythonTimerReactor(const object& timerFactory, object period) {
    auto properTimerFactory =
      [timerFactory = SharedObject(timerFactory)] (time_duration expiry) {
        auto gil = GilLock();
        auto lock = std::lock_guard(gil);
        return MakeVirtualTimer(extract<std::shared_ptr<VirtualTimer>>(
          (*timerFactory)(expiry))());
      };
    return new Aspen::Shared(Aspen::Box(TimerReactor<std::int64_t>(
      properTimerFactory, PythonReactor<time_duration>(std::move(period)))));
  }

  auto MakePythonDefaultTimerReactor(object period) {
    auto timerFactory =
      [] (time_duration duration) {
        return std::make_unique<LiveTimer>(duration,
          Ref(*GetTimerThreadPool()));
      };
    return new Aspen::Shared(Aspen::Box(TimerReactor<std::int64_t>(timerFactory,
      PythonReactor<time_duration>(std::move(period)))));
  }
}

#if 0
void Beam::Python::ExportAlarmReactor() {
  def("AlarmReactor", &MakePythonAlarmReactor);
  def("AlarmReactor", &MakePythonDefaultAlarmReactor);
  def("alarm", &MakePythonAlarmReactor);
  def("alarm", &MakePythonDefaultAlarmReactor);
}

void Beam::Python::ExportCurrentTimeReactor() {
  def("CurrentTimeReactor", &MakePythonCurrentTimeReactor);
  def("CurrentTimeReactor", &MakePythonDefaultCurrentTimeReactor);
  def("current_time", &MakePythonCurrentTimeReactor);
  def("current_time", &MakePythonDefaultCurrentTimeReactor);
}

void Beam::Python::ExportPublisherReactor() {
  def("PublisherReactor", &MakePythonPublisherReactor);
  def("publisher", &MakePythonPublisherReactor);
}

void Beam::Python::ExportQueryReactor() {
  ExportFunction<NoThrowFunction<void, const Queries::BasicQuery<object>&,
    std::shared_ptr<QueueWriter<object>>>>("QuerySubmissionFunction");
  def("QueryReactor", &MakePythonQueryReactor);
  def("query", &MakePythonQueryReactor);
  def("query_current", &MakePythonCurrentQueryReactor);
  def("query_real_time", &MakePythonRealTimeQueryReactor);
}

void Beam::Python::ExportQueueReactor() {
  using ExportedReactor = PythonQueueReactor;
  class_<ExportedReactor, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ExportedReactor>>("QueueReactor",
    init<std::shared_ptr<QueueReader<object>>>());
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<BaseReactor>>();
  def("queue", &MakePythonQueueReactor);
}
#endif

void Beam::Python::ExportReactors() {
  string nestedName = extract<string>(scope().attr("__name__") + ".reactors");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("reactors") = nestedModule;
  scope parent = nestedModule;
/*
  ExportAlarmReactor();
  ExportCurrentTimeReactor();
  ExportPublisherReactor();
  ExportQueryReactor();
  ExportQueueReactor();
*/
  ExportReactor<Aspen::Box<std::int64_t>>("Int64Box");
  ExportTimerReactor();
}

void Beam::Python::ExportTimerReactor() {
  def("TimerReactor", &MakePythonTimerReactor,
    return_value_policy<manage_new_object>());
  def("TimerReactor", &MakePythonDefaultTimerReactor,
    return_value_policy<manage_new_object>());
  def("timer", &MakePythonTimerReactor,
    return_value_policy<manage_new_object>());
  def("timer", &MakePythonDefaultTimerReactor,
    return_value_policy<manage_new_object>());
}
