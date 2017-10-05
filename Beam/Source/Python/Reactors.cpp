#include "Beam/Python/Reactors.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Reactors/BaseReactor.hpp"
#include "Beam/Reactors/Event.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/ReactorContainer.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"
#include "Beam/Reactors/TimerReactor.hpp"
#include "Beam/Reactors/Trigger.hpp"
#include "Beam/Python/SignalsSlots.hpp"
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
      const std::shared_ptr<Reactor<object>>& periodReactor) {
    auto pythonTimerFactory =
      [=] (const time_duration& duration) {
        VirtualTimer* result = extract<VirtualTimer*>(timerFactory(duration));
        return MakeVirtualTimer<VirtualTimer*>(std::move(result));
      };
    return MakeTimerReactor<std::int64_t>(pythonTimerFactory, periodReactor);
  }
}

#ifdef _MSC_VER
namespace boost {
  template<> inline const volatile BaseReactorWrapper* get_pointer(
      const volatile BaseReactorWrapper* p) {
    return p;
  }

  template<> inline const volatile ConstantReactor<object>* get_pointer(
      const volatile ConstantReactor<object>* p) {
    return p;
  }

  template<> inline const volatile EventWrapper* get_pointer(
      const volatile EventWrapper* p) {
    return p;
  }

  template<> inline const volatile Reactor<object>* get_pointer(
      const volatile Reactor<object>* p) {
    return p;
  }

  template<> inline const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<object>>* get_pointer(
      const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<object>>* p) {
    return p;
  }

  template<> inline const volatile std::type_info* get_pointer(
      const volatile std::type_info* p) {
    return p;
  }
}
#endif

void Beam::Python::ExportBaseReactor() {
  class_<BaseReactorWrapper, boost::noncopyable,
    std::shared_ptr<BaseReactorWrapper>>("BaseReactor")
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
  ExportSignal<>("EmptySignal");
}

void Beam::Python::ExportEvent() {
  class_<EventWrapper, boost::noncopyable, std::shared_ptr<EventWrapper>>(
    "Event", init<>())
    .def("execute", &Event::Execute, &EventWrapper::DefaultExecute)
    .def("connect_event_signal", &Event::ConnectEventSignal)
    .def("signal_event", &EventWrapper::SignalEvent);
  ExportSignal<>("EmptySignal");
}

void Beam::Python::ExportPythonConstantReactor() {
  class_<ConstantReactor<object>, bases<Reactor<object>>, boost::noncopyable,
    std::shared_ptr<ConstantReactor<object>>>("ConstantReactor",
    init<const object&>());
}

void Beam::Python::ExportPythonReactorContainer() {
  using PythonReactorContainer =
    ReactorContainer<std::shared_ptr<Reactor<object>>>;
  class_<PythonReactorContainer, boost::noncopyable>("ReactorContainer",
    init<const std::shared_ptr<Reactor<object>>&,
    const BaseReactor::UpdateSignal::slot_type&>())
    .def(init<const std::shared_ptr<Reactor<object>>&, BaseReactor&>())
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
  ExportReactor<Reactor<object>>("Reactor");
  ExportReactorMonitor();
  ExportTimerReactor();
  ExportTrigger();
  ExportPythonConstantReactor();
  ExportPythonReactorContainer();
}

void Beam::Python::ExportTimerReactor() {
  def("make_timer_reactor", &MakePythonTimerReactor);
}

void Beam::Python::ExportTrigger() {
  class_<Trigger, noncopyable>("Trigger", no_init)
    .def("__init__", make_constructor(&MakeTrigger))
    .def("__del__", BlockingFunction(&DeleteTrigger))
    .def("do", BlockingFunction(&Trigger::Do));
}
