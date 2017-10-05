#ifndef BEAM_PYTHON_REACTORS_HPP
#define BEAM_PYTHON_REACTORS_HPP
#include <boost/python.hpp>
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/Python.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
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

  template<typename T>
  T Converter(const boost::python::object& object) {
    GilLock gil;
    boost::lock_guard<GilLock> lock{gil};
    const T& value = boost::python::extract<const T&>(object);
    return value;
  }

  template<typename T>
  std::shared_ptr<Reactors::Reactor<T>> MakeFromPythonReactor(
      const std::shared_ptr<Reactors::Reactor<boost::python::object>>&
      reactor) {
    return Reactors::MakeFunctionReactor(&Converter<T>, reactor);
  }

  template<typename T>
  struct ReactorFromPythonConverter {
    static T Converter(const boost::python::object& object) {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      const T& value = boost::python::extract<const T&>(object);
      return value;
    }

    static void* convertible(PyObject* object) {
      boost::python::handle<> handle{object};
      boost::python::object reactor{handle};
      boost::python::extract<std::shared_ptr<Reactors::Reactor<
        boost::python::object>>> extractor{reactor};
      if(extractor.check()) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      using PythonReactor = Reactors::FunctionReactor<
        T (*)(const boost::python::object&),
        std::shared_ptr<Reactors::Reactor<boost::python::object>>>;
      auto storage = reinterpret_cast<boost::python::converter::
        rvalue_from_python_storage<PythonReactor>*>(data)->storage.bytes;
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object value{handle};
      std::shared_ptr<Reactor<boost::python::object>> reactor =
        boost::python::extract<std::shared_ptr<Reactor<boost::python::object>>>(
        value);
      new(storage) PythonReactor{&Converter, reactor};
      data->convertible = storage;
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
    boost::python::converter::registry::push_back(
      &Details::ReactorFromPythonConverter<typename T::Type>::convertible,
      &Details::ReactorFromPythonConverter<typename T::Type>::construct,
      boost::python::type_id<T>());
  }
}
}

#endif
