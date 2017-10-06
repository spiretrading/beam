#ifndef BEAM_PYTHON_REACTORS_HPP
#define BEAM_PYTHON_REACTORS_HPP
#include <boost/core/demangle.hpp>
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
      if(auto f = this->get_override("commit")) {
        f();
        return;
      }
      T::Commit();
    }

    void DefaultCommit() {
      this->T::Commit();
    }

    virtual Expect<void> GetBaseValue() override {
      if(auto f = this->get_override("get_base_value")) {
        return f();
      }
      return T::GetBaseValue();
    }

    Expect<void> DefaultGetBaseValue() {
      return T::GetBaseValue();
    }

    virtual const std::type_info& GetType() const override {
      if(auto f = this->get_override("get_type")) {
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
  T FromPythonConverter(const boost::python::object& object) {
    GilLock gil;
    boost::lock_guard<GilLock> lock{gil};
    boost::python::extract<T> value{object};
    if(value.check()) {
      return static_cast<T>(value);
    }
    throw Reactors::ReactorError{"Expected object of type: " +
      boost::core::demangle(typeid(T).name())};
  }

  template<typename T>
  boost::python::object ToPythonConverter(const T& object) {
    boost::python::object value{object};
    return value;
  }

  template<typename T>
  struct ReactorToPython {
    static PyObject* convert(const std::shared_ptr<T>& reactor) {
      std::shared_ptr<Reactors::Reactor<boost::python::object>> pythonReactor =
        Reactors::MakeFunctionReactor(&ToPythonConverter<typename T::Type>,
        reactor);
      boost::python::object value{pythonReactor};
      boost::python::incref(value.ptr());
      return value.ptr();
    }
  };

  template<typename T>
  struct ReactorFromPythonConverter {
    static void* convertible(PyObject* object) {
      boost::python::handle<> handle{boost::python::borrowed(object)};
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
      using ConversionReactor = Reactors::FunctionReactor<
        typename T::Type (*)(const boost::python::object&),
        std::shared_ptr<Reactors::Reactor<boost::python::object>>>;
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<
        std::shared_ptr<T>>*>(data)->storage.bytes;
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object value{handle};
      std::shared_ptr<Reactors::Reactor<boost::python::object>> reactor =
        boost::python::extract<std::shared_ptr<
        Reactors::Reactor<boost::python::object>>>(value);
      new(storage) std::shared_ptr<T>{std::make_shared<ConversionReactor>(
        &FromPythonConverter<typename T::Type>, reactor)};
      data->convertible = storage;
    }
  };
}

  //! A Reactor that evaluates to Python objects.
  using PythonReactor = Reactors::Reactor<boost::python::object>;

  //! Exports the AggregateReactor class.
  void ExportAggregateReactor();

  //! Exports the AlarmReactor.
  void ExportAlarmReactor();

  //! Exports the BaseReactor class.
  void ExportBaseReactor();

  //! Exports the Do Reactor.
  void ExportDoReactor();

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

  //! Makes a Reactor<T> from a Python Reactor.
  /*!
    \param reactor The Python Reactor to convert.
    \return A Reactor that converts Python objects to objects of type T.
  */
  template<typename T>
  std::shared_ptr<Reactors::Reactor<T>> MakeFromPythonReactor(
      const std::shared_ptr<PythonReactor>& reactor) {
    return Reactors::MakeFunctionReactor(
      &Details::FromPythonConverter<T>, reactor);
  }

  //! Makes a Python Reactor from a Reactor<T>.
  /*!
    \param reactor The Reactor to convert.
    \return A Reactor that converts from objects of type T to Python objects.
  */
  template<typename T>
  std::shared_ptr<PythonReactor> MakeToPythonReactor(
      const std::shared_ptr<Reactors::Reactor<T>>& reactor) {
    return Reactors::MakeFunctionReactor(&Details::ToPythonConverter<T>,
      reactor);
  }

  //! Exports the Reactor class template.
  /*!
    \param name The name of the class.
  */
  template<typename T>
  void ExportReactor(const char* name) {
    auto typeId = boost::python::type_id<T>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::class_<Details::ReactorWrapper<T>,
      std::shared_ptr<Details::ReactorWrapper<T>>, boost::noncopyable,
      boost::python::bases<Reactors::BaseReactor>>(name)
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
    if(!std::is_same<T, PythonReactor>::value) {
      boost::python::to_python_converter<std::shared_ptr<T>,
        Details::ReactorToPython<T>>();
      boost::python::converter::registry::push_back(
        &Details::ReactorFromPythonConverter<T>::convertible,
        &Details::ReactorFromPythonConverter<T>::construct,
        boost::python::type_id<std::shared_ptr<T>>());
    } else {
      boost::python::register_ptr_to_python<std::shared_ptr<T>>();
    }
    boost::python::implicitly_convertible<
      std::shared_ptr<Details::ReactorWrapper<T>>, std::shared_ptr<T>>();
    boost::python::implicitly_convertible<
      std::shared_ptr<Details::ReactorWrapper<T>>,
      std::shared_ptr<BaseReactor>>();
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<BaseReactor>>();
  }
}
}

#endif
