#ifndef BEAM_PYTHON_REACTORS_HPP
#define BEAM_PYTHON_REACTORS_HPP
#include <boost/core/demangle.hpp>
#include <boost/python.hpp>
#include "Beam/Python/Python.hpp"
#include "Beam/Python/FromPythonReactor.hpp"
#include "Beam/Python/ToPythonReactor.hpp"
#include "Beam/Reactors/Expressions.hpp"
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
  struct ReactorToPython {
    static PyObject* convert(
        const std::shared_ptr<Reactors::Reactor<T>>& reactor) {
      auto pythonReactor =
        [&] {
          if(auto pythonReactor = std::dynamic_pointer_cast<
              Reactors::FromPythonReactor<T>>(reactor)) {
            return pythonReactor->GetReactor();
          }
          return std::static_pointer_cast<
            Reactors::Reactor<boost::python::object>>(
            Reactors::MakeToPythonReactor(reactor));
        }();
      return boost::python::incref(boost::python::object{pythonReactor}.ptr());
    }
  };

  template<typename T>
  struct ReactorFromPythonConverter {
    static void* convertible(PyObject* object) {
      if(boost::python::extract<std::shared_ptr<
          Reactors::Reactor<boost::python::object>>>{object}.check()) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      auto reactor = boost::python::extract<std::shared_ptr<
        Reactors::Reactor<boost::python::object>>>{object}();
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<
        std::shared_ptr<Reactors::Reactor<T>>>*>(data)->storage.bytes;
      if(auto wrapperReactor = std::dynamic_pointer_cast<
          Reactors::ToPythonReactor<T>>(reactor)) {
        new(storage) std::shared_ptr<Reactors::Reactor<T>>{
          wrapperReactor->GetReactor()};
      } else {
        new(storage) std::shared_ptr<Reactors::Reactor<T>>{
          std::make_shared<Reactors::FromPythonReactor<T>>(std::move(reactor))};
      }
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

  //! Exports the FunctionReactor.
  void ExportFunctionReactor();

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
    auto add = &Reactors::Add<typename T::Type, boost::python::object>;
    auto sub = &Reactors::Subtract<typename T::Type, boost::python::object>;
    auto mul = &Reactors::Multiply<typename T::Type, boost::python::object>;
    auto div = &Reactors::Divide<typename T::Type, boost::python::object>;
    auto lt = &Reactors::Less<typename T::Type, boost::python::object>;
    auto le = &Reactors::LessOrEqual<typename T::Type, boost::python::object>;
    auto ge = &Reactors::GreaterOrEqual<
      typename T::Type, boost::python::object>;
    auto gt = &Reactors::Greater<typename T::Type, boost::python::object>;
    boost::python::class_<Details::ReactorWrapper<T>,
      std::shared_ptr<Details::ReactorWrapper<T>>, boost::noncopyable,
      boost::python::bases<Reactors::BaseReactor>>(name, boost::python::no_init)
      .def("eval", boost::python::pure_virtual(&T::Eval))
      .def("__add__", add)
      .def("__sub__", sub)
      .def("__mul__", mul)
      .def("__truediv__", div)
      .def("__lt__", lt)
      .def("__le__", le)
      .def("__ge__", ge)
      .def("__gt__", gt);
    if(!std::is_same<T, PythonReactor>::value) {
      boost::python::to_python_converter<std::shared_ptr<T>,
        Details::ReactorToPython<typename T::Type>>();
      boost::python::converter::registry::push_back(
        &Details::ReactorFromPythonConverter<typename T::Type>::convertible,
        &Details::ReactorFromPythonConverter<typename T::Type>::construct,
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
