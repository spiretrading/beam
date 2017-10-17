#include "Beam/Python/Reactors.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/SignalsSlots.hpp"
#include "Beam/Python/Tuple.hpp"
#include "Beam/Reactors/AlarmReactor.hpp"
#include "Beam/Reactors/BaseReactor.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/DoReactor.hpp"
#include "Beam/Reactors/ReactorError.hpp"
#include "Beam/Reactors/ReactorException.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"
#include "Beam/Reactors/TimerReactor.hpp"
#include "Beam/Reactors/Trigger.hpp"
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

namespace {
  struct BaseReactorWrapper : BaseReactor, wrapper<BaseReactor> {
    virtual bool IsComplete() const override final {
      return this->get_override("is_complete");
    }

    virtual const std::type_info& GetType() const override final {
      return typeid(boost::python::object);
    }

    virtual Update Commit(int sequenceNumber) override final {
      return this->get_override("commit")(sequenceNumber);
    }
  };

  auto MakePythonAlarmReactor(const object& timerFactory,
      VirtualTimeClient* timeClient,
      const std::shared_ptr<Reactor<ptime>>& expiryReactor,
      Trigger& trigger) {
    auto pythonTimerFactory =
      [=] (const time_duration& duration) {
        VirtualTimer* result = extract<VirtualTimer*>(timerFactory(duration));
        return MakeVirtualTimer<VirtualTimer*>(std::move(result));
      };
    return static_pointer_cast<Reactor<bool>>(
      MakeAlarmReactor(pythonTimerFactory, timeClient, expiryReactor,
      Ref(trigger)));
  }

  auto MakePythonDefaultAlarmReactor(
      const std::shared_ptr<Reactor<ptime>>& expiryReactor, Trigger& trigger) {
    auto pythonTimerFactory =
      [=] (const time_duration& duration) {
        auto timer = std::make_unique<LiveTimer>(duration,
          Ref(*GetTimerThreadPool()));
        return timer;
      };
    return static_pointer_cast<Reactor<bool>>(MakeAlarmReactor(
      pythonTimerFactory, std::make_shared<LocalTimeClient>(), expiryReactor,
      Ref(trigger)));
  }

  auto MakePythonTimerReactor(const object& timerFactory,
      const std::shared_ptr<Reactor<time_duration>>& periodReactor,
      Trigger* trigger) {
    auto pythonTimerFactory =
      [=] (const time_duration& duration) {
        VirtualTimer* result = extract<VirtualTimer*>(timerFactory(duration));
        return MakeVirtualTimer<VirtualTimer*>(std::move(result));
      };
    return MakeTimerReactor<std::int64_t>(pythonTimerFactory,
      periodReactor, Ref(*trigger));
  }

  auto MakePythonDefaultTimerReactor(
      const std::shared_ptr<Reactor<time_duration>>& periodReactor,
      Trigger* trigger) {
    auto pythonTimerFactory =
      [=] (const time_duration& duration) {
        auto timer = std::make_unique<LiveTimer>(duration,
          Ref(*GetTimerThreadPool()));
        return timer;
      };
    return MakeTimerReactor<std::int64_t>(pythonTimerFactory,
      periodReactor, Ref(*trigger));
  }

  std::shared_ptr<PythonReactor> MakePythonDoReactor(const object& callable,
      const std::shared_ptr<PythonReactor>& reactor) {

    // TODO: We need to lock the GIL before the parameter Expect<object> is
    // constructed.
    return Do(
      [=] (const Expect<object>& value) {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        try {
          callable(value);
        } catch(const boost::python::error_already_set&) {
          PrintError();
        }
      }, reactor);
  }

  void ReactorMonitorDo(ReactorMonitor* monitor, const object& callable) {
    monitor->Do(
      [=] {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        try {
          callable();
        } catch(const boost::python::error_already_set&) {
          PrintError();
        }
      });
  }

  int TriggerSignalUpdate(Trigger* trigger) {
    int sequenceNumber;
    trigger->SignalUpdate(Store(sequenceNumber));
    return sequenceNumber;
  }
}

#ifdef _MSC_VER
namespace boost {
  template<> inline const volatile BaseReactor* get_pointer(
      const volatile BaseReactor* p) {
    return p;
  }

  template<> inline const volatile BaseReactorWrapper* get_pointer(
      const volatile BaseReactorWrapper* p) {
    return p;
  }

  template<> inline const volatile ConstantReactor<object>* get_pointer(
      const volatile ConstantReactor<object>* p) {
    return p;
  }

  template<> inline const volatile Publisher<int>* get_pointer(
      const volatile Publisher<int>* p) {
    return p;
  }

  template<> inline const volatile PythonReactor* get_pointer(
      const volatile PythonReactor* p) {
    return p;
  }

  template<> inline const volatile Reactor<bool>* get_pointer(
      const volatile Reactor<bool>* p) {
    return p;
  }

  template<> inline const volatile Reactor<std::int64_t>* get_pointer(
      const volatile Reactor<std::int64_t>* p) {
    return p;
  }

  template<> inline const volatile Reactor<ptime>* get_pointer(
      const volatile Reactor<ptime>* p) {
    return p;
  }

  template<> inline const volatile Reactor<std::shared_ptr<Reactor<object>>>*
      get_pointer(const volatile Reactor<std::shared_ptr<Reactor<object>>>* p) {
    return p;
  }

  template<> inline const volatile Reactor<time_duration>* get_pointer(
      const volatile Reactor<time_duration>* p) {
    return p;
  }

  template<> inline const volatile Beam::Python::Details::ReactorWrapper<
      PythonReactor>* get_pointer(
      const volatile Beam::Python::Details::ReactorWrapper<PythonReactor>* p) {
    return p;
  }

  template<> inline const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<bool>>* get_pointer(
      const volatile Beam::Python::Details::ReactorWrapper<Reactor<bool>>* p) {
    return p;
  }

  template<> inline const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<ptime>>* get_pointer(
      const volatile Beam::Python::Details::ReactorWrapper<Reactor<ptime>>* p) {
    return p;
  }

  template<> inline const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<std::shared_ptr<Reactor<object>>>>* get_pointer(
      const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<std::shared_ptr<Reactor<object>>>>* p) {
    return p;
  }

  template<> inline const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<time_duration>>* get_pointer(
      const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<time_duration>>* p) {
    return p;
  }

  template<> inline const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<std::int64_t>>* get_pointer(
      const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<std::int64_t>>* p) {
    return p;
  }

  template<> inline const volatile std::type_info* get_pointer(
      const volatile std::type_info* p) {
    return p;
  }
}
#endif

void Beam::Python::ExportAlarmReactor() {
  def("AlarmReactor", &MakePythonAlarmReactor);
  def("AlarmReactor", &MakePythonDefaultAlarmReactor);
}

void Beam::Python::ExportBaseReactor() {
  class_<BaseReactorWrapper, std::shared_ptr<BaseReactorWrapper>,
    boost::noncopyable>("BaseReactor")
    .def("is_complete", pure_virtual(&BaseReactor::IsComplete))
    .def("commit", pure_virtual(&BaseReactor::Commit));
  register_ptr_to_python<std::shared_ptr<BaseReactor>>();
  implicitly_convertible<std::shared_ptr<BaseReactorWrapper>,
    std::shared_ptr<BaseReactor>>();
}

void Beam::Python::ExportDoReactor() {
  def("do", &MakePythonDoReactor);
}

void Beam::Python::ExportPythonConstantReactor() {
  class_<ConstantReactor<object>, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ConstantReactor<object>>>("ConstantReactor",
    init<const object&>());
  implicitly_convertible<std::shared_ptr<ConstantReactor<object>>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ConstantReactor<object>>,
    std::shared_ptr<BaseReactor>>();
}

void Beam::Python::ExportReactorMonitor() {
  class_<ReactorMonitor, noncopyable>("ReactorMonitor", init<>())
    .add_property("trigger", make_function(
      static_cast<Trigger& (ReactorMonitor::*)()>(&ReactorMonitor::GetTrigger),
      return_internal_reference<>()))
    .def("add", BlockingFunction(&ReactorMonitor::Add))
    .def("do", &ReactorMonitorDo)
    .def("open", BlockingFunction(&ReactorMonitor::Open))
    .def("close", BlockingFunction(&ReactorMonitor::Close));
}

void Beam::Python::ExportReactors() {
  string nestedName = extract<string>(scope().attr("__name__") + ".reactors");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("reactors") = nestedModule;
  scope parent = nestedModule;
  ExportBaseReactor();
  ExportReactor<PythonReactor>("Reactor");
  ExportReactorMonitor();
  ExportDoReactor();
  ExportAlarmReactor();
  ExportTimerReactor();
  ExportTrigger();
  ExportPythonConstantReactor();
  ExportReactor<Reactor<std::shared_ptr<PythonReactor>>>("MetaReactor");
  ExportReactor<Reactor<ptime>>("DateReactor");
  ExportReactor<Reactor<bool>>("BoolReactor");
  ExportReactor<Reactor<time_duration>>("TimeDurationReactor");
  ExportReactor<Reactor<std::int64_t>>("Int64Reactor");
  ExportException<ReactorException, std::runtime_error>("ReactorException")
    .def(init<>())
    .def(init<const string&>());
  ExportException<ReactorError, ReactorException>("ReactorError")
    .def(init<>())
    .def(init<const string&>());
  ExportException<ReactorUnavailableException, ReactorException>(
    "ReactorUnavailableException")
    .def(init<>())
    .def(init<const string&>());
}

void Beam::Python::ExportTimerReactor() {
  def("TimerReactor", &MakePythonTimerReactor);
  def("TimerReactor", &MakePythonDefaultTimerReactor);
}

void Beam::Python::ExportTrigger() {
  class_<Trigger, noncopyable>("Trigger", init<>())
    .def("signal_update", &TriggerSignalUpdate)
    .add_property("sequence_number_publisher", make_function(
      &Trigger::GetSequenceNumberPublisher, return_internal_reference<>()));
}
