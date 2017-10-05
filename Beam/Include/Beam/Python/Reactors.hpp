#ifndef BEAM_PYTHON_REACTORS_HPP
#define BEAM_PYTHON_REACTORS_HPP
#include <boost/python.hpp>
#include "Beam/Python/Python.hpp"
#include "Beam/Reactors/Reactor.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename T>
  struct ReactorWrapper : T, boost::python::wrapper<T> {
    using Type = typename T::Type;

    virtual void Commit() override {
      if(override f = this->get_override("commit")) {
        f();
        return;
      }
      T::Commit();
    }

    void DefaultCommit() {
      this->T::Commit();
    }

    virtual Expect<void> GetBaseValue() override {
      if(override f = this->get_override("get_base_value")) {
        return f();
      }
      return T::GetBaseValue();
    }

    Expect<void> DefaultGetBaseValue() {
      return T::GetBaseValue();
    }

    virtual const std::type_info& GetType() const override {
      if(override f = this->get_override("get_type")) {
        return *static_cast<const std::type_info*>(f());
      }
      return T::GetType();
    }

    const std::type_info& DefaultGetType() const {
      return T::GetType();
    }

    virtual Type Eval() const override {
      return this->get_override("eval")();
    }

    void IncrementSequenceNumber() {
      T::IncrementSequenceNumber();
    }

    void SetComplete() {
      T::SetComplete();
    }

    void SignalUpdate() {
      T::SignalUpdate();
    }
  };
}

  //! Exports the BaseReactor class.
  void ExportBaseReactor();

  //! Exports the Event class.
  void ExportEvent();

  //! Exports a ConstantReactor<object> class.
  void ExportPythonConstantReactor();

  //! Exports a ReactorContainer for a Python object.
  void ExportPythonReactorContainer();

  //! Exports the ReactorMonitor class.
  void ExportReactorMonitor();

  //! Exports the Reactors namespace.
  void ExportReactors();

  //! Exports the TimerReactor class.
  void ExportTimerReactor();

  //! Exports the Trigger class.
  void ExportTrigger();

  //! Exports the Reactor template class.
  template<typename T>
  void ExportReactor(const char* name) {
    auto typeId = boost::python::type_id<T>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::class_<Details::ReactorWrapper<T>, boost::noncopyable,
      boost::python::bases<Reactors::BaseReactor>,
      std::shared_ptr<Details::ReactorWrapper<T>>>(
      name, boost::python::init<>())
    .def("commit", &T::Commit, &Details::ReactorWrapper<T>::DefaultCommit)
    .def("get_base_value", &T::GetBaseValue,
      &Details::ReactorWrapper<T>::DefaultGetBaseValue)
    .def("get_type", &T::GetType, &Details::ReactorWrapper<T>::DefaultGetType,
      boost::python::return_value_policy<
      boost::python::reference_existing_object>())
    .def("eval", pure_virtual(&T::Eval))
    .def("_increment_sequence_number",
      &Details::ReactorWrapper<T>::IncrementSequenceNumber)
    .def("_set_complete", &Details::ReactorWrapper<T>::SetComplete)
    .def("_signal_update", &Details::ReactorWrapper<T>::SignalUpdate);
  }
}
}

#endif
