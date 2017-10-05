#include "Beam/Python/Reactors.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/SignalsSlots.hpp"
#include "Beam/Python/Tuple.hpp"
#include "Beam/Reactors/BaseReactor.hpp"
#include "Beam/Reactors/Control.hpp"
#include "Beam/Reactors/Event.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/ReactorContainer.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"
#include "Beam/Reactors/TimerReactor.hpp"
#include "Beam/Reactors/Trigger.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Threading/VirtualTimer.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Reactors;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace std;

namespace {
  struct BaseReactorWrapper : BaseReactor, wrapper<BaseReactor> {
    virtual void Commit() override {
      this->get_override("commit")();
    }

    virtual Expect<void> GetBaseValue() override {
      return this->get_override("get_base_value")();
    }

    virtual const std::type_info& GetType() const override {
      return *static_cast<const std::type_info*>(this->get_override(
        "get_type")());
    }

    void IncrementSequenceNumber() {
      BaseReactor::IncrementSequenceNumber();
    }

    void SetComplete() {
      BaseReactor::SetComplete();
    }

    void SignalUpdate() {
      BaseReactor::SignalUpdate();
    }
  };

  struct EventWrapper : Event, wrapper<Event> {
    virtual void Execute() override {
      if(override f = this->get_override("execute")) {
        f();
        return;
      }
      Event::Execute();
    }

    void DefaultExecute() {
      this->Event::Execute();
    }

    void SignalEvent() {
      Event::SignalEvent();
    }
  };

  void DeleteReactorMonitor(ReactorMonitor& monitor) {
    monitor.Close();
  }

  Trigger* MakeTrigger(ReactorMonitor& monitor) {
    GilRelease gil;
    boost::lock_guard<GilRelease> lock{gil};
    return new Trigger{monitor};
  }

  void DeleteTrigger(Trigger& trigger) {
    trigger.~Trigger();
  }

  auto MakePythonTimerReactor(const object& timerFactory,
      const std::shared_ptr<PythonReactor>& periodReactor) {
    auto pythonTimerFactory =
      [=] (const time_duration& duration) {
        VirtualTimer* result = extract<VirtualTimer*>(timerFactory(duration));
        return MakeVirtualTimer<VirtualTimer*>(std::move(result));
      };
    auto result = MakeTimerReactor<std::int64_t>(pythonTimerFactory,
      MakeFromPythonReactor<time_duration>(periodReactor));
    return std::make_tuple(MakeToPythonReactor(std::get<0>(result)),
      std::get<1>(result));
  }

  auto MakePythonDefaultTimerReactor(
      const std::shared_ptr<PythonReactor>& periodReactor) {
    auto pythonTimerFactory =
      [=] (const time_duration& duration) {
        auto timer = std::make_unique<LiveTimer>(duration,
          Ref(*GetTimerThreadPool()));
        return timer;
      };
    auto result = MakeTimerReactor<std::int64_t>(pythonTimerFactory,
      MakeFromPythonReactor<time_duration>(periodReactor));
    return std::make_tuple(MakeToPythonReactor(std::get<0>(result)),
      std::get<1>(result));
  }

  std::shared_ptr<PythonReactor> MakePythonDoReactor(const object& callable,
      const std::shared_ptr<PythonReactor>& reactor) {
    return Do(
      [=] (const Expect<object>& value) {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        return callable(value);
      }, reactor);
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

  template<> inline const volatile Event* get_pointer(const volatile Event* p) {
    return p;
  }

  template<> inline const volatile EventWrapper* get_pointer(
      const volatile EventWrapper* p) {
    return p;
  }

  template<> inline const volatile PythonReactor* get_pointer(
      const volatile PythonReactor* p) {
    return p;
  }

  template<> inline const volatile Reactor<std::int64_t>* get_pointer(
      const volatile Reactor<std::int64_t>* p) {
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

void Beam::Python::ExportBaseReactor() {
  class_<BaseReactorWrapper, std::shared_ptr<BaseReactorWrapper>,
    boost::noncopyable>("BaseReactor")
    .add_property("sequence_number", &BaseReactor::GetSequenceNumber)
    .add_property("is_initializing", &BaseReactor::IsInitializing)
    .add_property("is_initialized", &BaseReactor::IsInitialized)
    .add_property("has_evaluation", &BaseReactor::HasEvaluation)
    .add_property("is_complete", &BaseReactor::IsComplete)
    .def("commit", pure_virtual(&BaseReactor::Commit))
    .def("get_base_value", pure_virtual(&BaseReactor::GetBaseValue))
    .def("get_type", pure_virtual(&BaseReactor::GetType),
      return_value_policy<reference_existing_object>())
    .def("connect_update_signal", &BaseReactor::ConnectUpdateSignal)
    .def("_increment_sequence_number",
      &BaseReactorWrapper::IncrementSequenceNumber)
    .def("_set_complete", &BaseReactorWrapper::SetComplete)
    .def("_signal_update", &BaseReactorWrapper::SignalUpdate);
  register_ptr_to_python<std::shared_ptr<BaseReactor>>();
  implicitly_convertible<std::shared_ptr<BaseReactorWrapper>,
    std::shared_ptr<BaseReactor>>();
  ExportSignal<>("EmptySignal");
}

void Beam::Python::ExportDoReactor() {
  def("do", &MakePythonDoReactor);
}

void Beam::Python::ExportEvent() {
  class_<EventWrapper, boost::noncopyable, std::shared_ptr<EventWrapper>>(
    "Event", init<>())
    .def("execute", &Event::Execute, &EventWrapper::DefaultExecute)
    .def("connect_event_signal", &Event::ConnectEventSignal)
    .def("signal_event", &EventWrapper::SignalEvent);
  register_ptr_to_python<std::shared_ptr<Event>>();
  implicitly_convertible<std::shared_ptr<EventWrapper>,
    std::shared_ptr<Event>>();
  ExportSignal<>("EmptySignal");
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

void Beam::Python::ExportPythonReactorContainer() {
  using PythonReactorContainer =
    ReactorContainer<std::shared_ptr<PythonReactor>>;
  class_<PythonReactorContainer, boost::noncopyable>("ReactorContainer",
    init<const std::shared_ptr<PythonReactor>&,
    const BaseReactor::UpdateSignal::slot_type&>())
    .def(init<const std::shared_ptr<PythonReactor>&, BaseReactor&>())
    .add_property("reactor", make_function(&PythonReactorContainer::GetReactor,
      return_internal_reference<>()))
    .add_property("is_initializing", &PythonReactorContainer::IsInitializing)
    .add_property("is_initialized", &PythonReactorContainer::IsInitialized)
    .add_property("is_complete", &PythonReactorContainer::IsComplete)
    .add_property("value", make_function(&PythonReactorContainer::GetValue,
      return_value_policy<copy_const_reference>()))
    .def("eval", &PythonReactorContainer::Eval,
      return_value_policy<copy_const_reference>())
    .def("commit", &PythonReactorContainer::Commit);
}

void Beam::Python::ExportReactorMonitor() {
  class_<ReactorMonitor, noncopyable>("ReactorMonitor", init<>())
    .def("__del__", BlockingFunction(&DeleteReactorMonitor))
    .def("add_event", BlockingFunction(&ReactorMonitor::AddEvent))
    .def("add_reactor", BlockingFunction(&ReactorMonitor::AddReactor))
    .def("connect_complete_signal",
      BlockingFunction(&ReactorMonitor::ConnectCompleteSignal))
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
  ExportEvent();
  ExportReactor<PythonReactor>("Reactor");
  ExportReactorMonitor();
  ExportDoReactor();
  ExportTimerReactor();
  ExportTrigger();
  ExportPythonConstantReactor();
  ExportPythonReactorContainer();
}

void Beam::Python::ExportTimerReactor() {
  def("TimerReactor", &MakePythonTimerReactor);
  def("TimerReactor", &MakePythonDefaultTimerReactor);
  ExportReactor<Reactor<time_duration>>("TimeDurationReactor");
  ExportReactor<Reactor<std::int64_t>>("Int64Reactor");
  ExportTuple<std::shared_ptr<PythonReactor>, std::shared_ptr<Event>>();
}

void Beam::Python::ExportTrigger() {
  class_<Trigger, noncopyable>("Trigger", no_init)
    .def("__init__", make_constructor(&MakeTrigger))
    .def("__del__", BlockingFunction(&DeleteTrigger))
    .def("do", BlockingFunction(&Trigger::Do));
}
