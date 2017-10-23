#ifndef BEAM_PYTHON_REACTORS_HPP
#define BEAM_PYTHON_REACTORS_HPP
#include <boost/core/demangle.hpp>
#include <boost/python.hpp>
#include "Beam/Python/Python.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactor.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename T>
  struct ReactorWrapper : T, boost::python::wrapper<T> {
    using Type = typename T::Type;

    virtual bool IsComplete() const override final {
      return this->get_override("is_complete")();
    }

    virtual const std::type_info& GetType() const override final {
      return typeid(boost::python::object);
    }

    virtual Reactors::BaseReactor::Update
        Commit(int sequenceNumber) override final {
      return this->get_override("commit")(sequenceNumber);
    }

    virtual Type Eval() const override {
      return this->get_override("eval")();
    }
  };

  template<typename T>
  T FromPythonConverter(const boost::python::object& object) {
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

  //! Exports the AlarmReactor class.
  void ExportAlarmReactor();

  //! Exports the BaseReactor class.
  void ExportBaseReactor();

  //! Exports the BasicReactor class.
  void ExportBasicReactor();

  //! Exports the ChainReactor class.
  void ExportChainReactor();

  //! Exports the Do Reactor.
  void ExportDoReactor();

  //! Exports expression Reactors.
  void ExportExpressionReactors();

  //! Exports the Filter Reactor.
  void ExportFilterReactor();

  //! Exports the Fold Reactor.
  void ExportFoldReactor();

  //! Exports the NoneReactor class.
  void ExportNoneReactor();

  //! Exports the NonRepeating Reactor.
  void ExportNonRepeatingReactor();

  //! Exports the ProxyReactor class.
  void ExportProxyReactor();

  //! Exports the Publisher Reactor.
  void ExportPublisherReactor();

  //! Exports a ConstantReactor<object> class.
  void ExportPythonConstantReactor();

  //! Exports the QueueReactor class.
  void ExportQueueReactor();

  //! Exports the Range Reactor.
  void ExportRangeReactor();

  //! Exports the ReactorMonitor class.
  void ExportReactorMonitor();

  //! Exports the Reactors namespace.
  void ExportReactors();

  //! Exports the Static Reactor.
  void ExportStaticReactor();

  //! Exports the SwitchReactor class.
  void ExportSwitchReactor();

  //! Exports the Throw Reactor.
  void ExportThrowReactor();

  //! Exports the TimerReactor class.
  void ExportTimerReactor();

  //! Exports the Trigger class.
  void ExportTrigger();

  //! Exports the WhenComplete Reactor.
  void ExportWhenCompleteReactor();

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

  //! Wraps a function returning a Reactor to a function returning a
  //! PythonWrappedReactor.
  /*!
    \param f The function to wrap.
    \return A Python callable that invokes <i>f</i>.
  */
  template<typename R, typename... Args>
  boost::python::object PythonWrapReactor(R (*f)(Args...)) {
    std::function<std::shared_ptr<PythonReactor> (Args...)> callable =
      [=] (Args... args) -> std::shared_ptr<PythonReactor> {
        return MakePythonWrapperReactor((*f)(std::forward<Args>(args)...));
      };
    using signature = boost::mpl::vector<
      std::shared_ptr<PythonReactor>, Args...>;
    return boost::python::make_function(callable,
      boost::python::default_call_policies(), signature());
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
      .def("is_complete", boost::python::pure_virtual(&T::IsComplete))
      .def("commit", boost::python::pure_virtual(&T::Commit))
      .def("eval", boost::python::pure_virtual(&T::Eval));
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
      std::shared_ptr<Reactors::BaseReactor>>();
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<Reactors::BaseReactor>>();
  }
}
}

#endif
