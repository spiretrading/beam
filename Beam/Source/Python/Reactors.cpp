#include "Beam/Python/Reactors.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/GilLock.hpp"
#include "Beam/Reactors/BaseReactor.hpp"
#include "Beam/Reactors/Event.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Python/SignalsSlots.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Reactors;
using namespace boost;
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

void Beam::Python::ExportPythonConstantReactor() {
  class_<ConstantReactor<object>, bases<Reactor<object>>, boost::noncopyable,
    std::shared_ptr<ConstantReactor<object>>>("ConstantReactor",
    init<const object&>());
}

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

void Beam::Python::ExportReactors() {
  string nestedName = extract<string>(scope().attr("__name__") + ".reactors");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("reactors") = nestedModule;
  scope parent = nestedModule;
  ExportBaseReactor();
  ExportEvent();
  ExportReactor<Reactor<object>>("Reactor");
  ExportPythonConstantReactor();
}
