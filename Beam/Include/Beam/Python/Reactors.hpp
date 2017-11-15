#ifndef BEAM_PYTHON_REACTORS_HPP
#define BEAM_PYTHON_REACTORS_HPP
#include <boost/core/demangle.hpp>
#include <boost/python.hpp>
#include "Beam/Python/Python.hpp"
#include "Beam/Python/FromPythonReactor.hpp"
#include "Beam/Python/ToPythonReactor.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/Expressions.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactor.hpp"

namespace Beam {
namespace Python {

  //! Extracts a Reactor from a Python object.
  /*!
    \param value The Python object to extract the Reactor from.
  */
  template<typename T>
  inline auto ExtractReactor(const boost::python::object& value) {
    boost::python::extract<std::shared_ptr<Reactors::Reactor<T>>> reactor{
      value};
    if(reactor.check()) {
      return reactor();
    }
    return std::static_pointer_cast<Reactors::Reactor<T>>(
      Reactors::MakeConstantReactor(boost::python::extract<T>{value}()));
  }

  inline auto ExtractReactor(const boost::python::object& value) {
    boost::python::extract<std::shared_ptr<
      Reactors::Reactor<boost::python::object>>> reactor{value};
    if(reactor.check()) {
      return reactor();
    }
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::MakeConstantReactor(value));
  }

namespace Details {
  template<typename T>
  struct ReactorWrapper : T, boost::python::wrapper<T> {
    using Type = typename T::Type;

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

  template<typename T>
  auto PythonAddReactor(std::shared_ptr<Reactors::Reactor<T>> self,
      const boost::python::object& rhs) {
    auto rightReactor = ExtractReactor(std::move(rhs));
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::Add(std::move(self), std::move(rightReactor)));
  }

  template<typename T>
  auto PythonReverseAddReactor(std::shared_ptr<Reactors::Reactor<T>> self,
      const boost::python::object& lhs) {
    auto leftReactor = ExtractReactor(std::move(lhs));
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::Add(std::move(leftReactor), std::move(self)));
  }

  template<typename T>
  auto PythonSubtractReactor(std::shared_ptr<Reactors::Reactor<T>> self,
      const boost::python::object& rhs) {
    auto rightReactor = ExtractReactor(std::move(rhs));
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::Subtract(std::move(self), std::move(rightReactor)));
  }

  template<typename T>
  auto PythonReverseSubtractReactor(std::shared_ptr<Reactors::Reactor<T>> self,
      const boost::python::object& lhs) {
    auto leftReactor = ExtractReactor(std::move(lhs));
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::Subtract(std::move(leftReactor), std::move(self)));
  }

  template<typename T>
  auto PythonMultiplyReactor(std::shared_ptr<Reactors::Reactor<T>> self,
      const boost::python::object& rhs) {
    auto rightReactor = ExtractReactor(std::move(rhs));
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::Multiply(std::move(self), std::move(rightReactor)));
  }

  template<typename T>
  auto PythonReverseMultiplyReactor(std::shared_ptr<Reactors::Reactor<T>> self,
      const boost::python::object& lhs) {
    auto leftReactor = ExtractReactor(std::move(lhs));
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::Multiply(std::move(leftReactor), std::move(self)));
  }

  template<typename T>
  auto PythonDivideReactor(std::shared_ptr<Reactors::Reactor<T>> self,
      const boost::python::object& rhs) {
    auto rightReactor = ExtractReactor(std::move(rhs));
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::Divide(std::move(self), std::move(rightReactor)));
  }

  template<typename T>
  auto PythonReverseDivideReactor(std::shared_ptr<Reactors::Reactor<T>> self,
      const boost::python::object& lhs) {
    auto leftReactor = ExtractReactor(std::move(lhs));
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::Divide(std::move(leftReactor), std::move(self)));
  }

  template<typename T, typename U>
  auto PythonLessReactor(std::shared_ptr<Reactors::Reactor<T>> lhs,
      std::shared_ptr<Reactors::Reactor<U>> rhs) {
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::Less(std::move(lhs), std::move(rhs)));
  }

  template<typename T, typename U>
  auto PythonLessOrEqualReactor(std::shared_ptr<Reactors::Reactor<T>> lhs,
      std::shared_ptr<Reactors::Reactor<U>> rhs) {
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::LessOrEqual(std::move(lhs), std::move(rhs)));
  }

  template<typename T, typename U>
  auto PythonEqualReactor(std::shared_ptr<Reactors::Reactor<T>> lhs,
      std::shared_ptr<Reactors::Reactor<U>> rhs) {
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::Equal(std::move(lhs), std::move(rhs)));
  }

  template<typename T, typename U>
  auto PythonNotEqualReactor(std::shared_ptr<Reactors::Reactor<T>> lhs,
      std::shared_ptr<Reactors::Reactor<U>> rhs) {
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::NotEqual(std::move(lhs), std::move(rhs)));
  }

  template<typename T, typename U>
  auto PythonGreaterOrEqualReactor(std::shared_ptr<Reactors::Reactor<T>> lhs,
      std::shared_ptr<Reactors::Reactor<U>> rhs) {
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::GreaterOrEqual(std::move(lhs), std::move(rhs)));
  }

  template<typename T, typename U>
  auto PythonGreaterReactor(std::shared_ptr<Reactors::Reactor<T>> lhs,
      std::shared_ptr<Reactors::Reactor<U>> rhs) {
    return std::static_pointer_cast<Reactors::Reactor<boost::python::object>>(
      Reactors::Greater(std::move(lhs), std::move(rhs)));
  }
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

  //! Exports the CurrentTimeReactor.
  void ExportCurrentTimeReactor();

  //! Exports the Do Reactor.
  void ExportDoReactor();

  //! Exports the Filter Reactor.
  void ExportFilterReactor();

  //! Exports the First Reactor.
  void ExportFirstReactor();

  //! Exports the Fold Reactor.
  void ExportFoldReactor();

  //! Exports the FunctionReactor.
  void ExportFunctionReactor();

  //! Exports the Last Reactor.
  void ExportLastReactor();

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

  //! Exports the QueryReactor.
  void ExportQueryReactor();

  //! Exports the Range Reactor.
  void ExportRangeReactor();

  //! Exports the ReactorMonitor class.
  void ExportReactorMonitor();

  //! Exports the Reactors namespace.
  void ExportReactors();

  //! Exports the SwitchReactor class.
  void ExportSwitchReactor();

  //! Exports the Throw Reactor.
  void ExportThrowReactor();

  //! Exports the TimerReactor class.
  void ExportTimerReactor();

  //! Exports the Trigger class.
  void ExportTrigger();

  //! Exports the UpdateReactor class.
  void ExportUpdateReactor();

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
    auto add = &Details::PythonAddReactor<typename T::Type>;
    auto radd = &Details::PythonReverseAddReactor<typename T::Type>;
    auto sub = &Details::PythonSubtractReactor<typename T::Type>;
    auto rsub = &Details::PythonReverseSubtractReactor<typename T::Type>;
    auto mul = &Details::PythonMultiplyReactor<typename T::Type>;
    auto rmul = &Details::PythonReverseMultiplyReactor<typename T::Type>;
    auto div = &Details::PythonDivideReactor<typename T::Type>;
    auto rdiv = &Details::PythonReverseDivideReactor<typename T::Type>;
    auto lt = &Details::PythonLessReactor<typename T::Type,
      boost::python::object>;
    auto le = &Details::PythonLessOrEqualReactor<typename T::Type,
      boost::python::object>;
    auto eq = &Details::PythonEqualReactor<typename T::Type,
      boost::python::object>;
    auto neq = &Details::PythonNotEqualReactor<typename T::Type,
      boost::python::object>;
    auto ge = &Details::PythonGreaterOrEqualReactor<
      typename T::Type, boost::python::object>;
    auto gt = &Details::PythonGreaterReactor<typename T::Type,
      boost::python::object>;
    boost::python::class_<Details::ReactorWrapper<T>,
      std::shared_ptr<Details::ReactorWrapper<T>>, boost::noncopyable,
      boost::python::bases<Reactors::BaseReactor>>(name, boost::python::no_init)
      .def("eval", boost::python::pure_virtual(&T::Eval))
      .def("__add__", add)
      .def("__radd__", radd)
      .def("__sub__", sub)
      .def("__rsub__", rsub)
      .def("__mul__", mul)
      .def("__rmul__", rmul)
      .def("__truediv__", div)
      .def("__truediv__", div)
      .def("__rtruediv__", rdiv)
      .def("__rtruediv__", rdiv)
      .def("__lt__", lt)
      .def("__le__", le)
      .def("__eq__", eq)
      .def("__neq__", neq)
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
