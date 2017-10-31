#ifndef BEAM_PYTHON_FUNCTION_HPP
#define BEAM_PYTHON_FUNCTION_HPP
#include <functional>
#include <boost/python.hpp>
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/NoThrowFunction.hpp"
#include "Beam/Python/Python.hpp"
#include "Beam/Python/SharedObject.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename F>
  struct FunctionToPython {};

  template<typename R, typename... Args>
  struct FunctionToPython<std::function<R (Args...)>> {
    static PyObject* convert(const std::function<R (Args...)>& f) {
      using signature = boost::mpl::vector<R, Args...>;
      return boost::python::incref(boost::python::make_function(f,
        boost::python::default_call_policies(), signature()).ptr());
    }
  };

  template<typename R, typename... Args>
  struct FunctionToPython<NoThrowFunction<R, Args...>> {
    static PyObject* convert(const NoThrowFunction<R, Args...>& f) {
      using signature = boost::mpl::vector<R, Args...>;
      return boost::python::incref(boost::python::make_function(f,
        boost::python::default_call_policies(), signature()).ptr());
    }
  };

  template<typename F>
  struct FunctionFromPythonConverter {};

  template<typename R, typename... Args>
  struct FunctionFromPythonConverter<std::function<R (Args...)>> {
    static void* convertible(PyObject* object) {
      if(PyCallable_Check(object)) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object f{handle};
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<
        std::function<R (Args...)>>*>(data)->storage.bytes;
      new(storage) std::function<R (Args...)>{
        [f = SharedObject{std::move(f)}] (Args... args) {
          GilLock gil;
          boost::lock_guard<GilLock> lock{gil};
          return boost::python::extract<R>((*f)(args...))();
        }
      };
      data->convertible = storage;
    }
  };

  template<typename... Args>
  struct FunctionFromPythonConverter<std::function<void (Args...)>> {
    static void* convertible(PyObject* object) {
      if(PyCallable_Check(object)) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object f{handle};
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<
        std::function<void (Args...)>>*>(data)->storage.bytes;
      new(storage) std::function<void (Args...)>{
        [f = SharedObject{std::move(f)}] (Args... args) {
          GilLock gil;
          boost::lock_guard<GilLock> lock{gil};
          (*f)(args...);
        }
      };
      data->convertible = storage;
    }
  };

  template<typename R, typename... Args>
  struct FunctionFromPythonConverter<NoThrowFunction<R, Args...>> {
    static void* convertible(PyObject* object) {
      if(PyCallable_Check(object)) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object f{handle};
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<
        NoThrowFunction<R, Args...>>*>(data)->storage.bytes;
      new(storage) NoThrowFunction<R, Args...>{std::move(f)};
      data->convertible = storage;
    }
  };

  template<typename... Args>
  struct FunctionFromPythonConverter<NoThrowFunction<void, Args...>> {
    static void* convertible(PyObject* object) {
      if(PyCallable_Check(object)) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object f{handle};
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<
        NoThrowFunction<void, Args...>>*>(data)->storage.bytes;
      new(storage) NoThrowFunction<void, Args...>{std::move(f)};
      data->convertible = storage;
    }
  };
}

  //! Exports a C++ function.
  /*!
    \param name The name to assign to the function type.
  */
  template<typename F>
  void ExportFunction(const char* name) {
    auto typeId = boost::python::type_id<F>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::to_python_converter<F, Details::FunctionToPython<F>>();
    boost::python::converter::registry::push_back(
      &Details::FunctionFromPythonConverter<F>::convertible,
      &Details::FunctionFromPythonConverter<F>::construct,
      boost::python::type_id<F>());
  }

  //! Exports some common function types.
  void ExportFunctions();
}
}

#endif
