#ifndef BEAM_PYTHONCOPY_HPP
#define BEAM_PYTHONCOPY_HPP
#include <cstdint>
#include <boost/python.hpp>
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {
  template<class T>
  PyObject* GetManagingObject(T* p) {
    return typename boost::python::manage_new_object::apply<T*>::type()(p);
  }

  template<class Copyable>
  boost::python::object MakeCopy(boost::python::object copyable) {
    auto newCopyable = new Copyable{boost::python::extract<const Copyable&>(
      copyable)};
    boost::python::object result{boost::python::detail::new_reference(
      GetManagingObject(newCopyable))};
    boost::python::extract<boost::python::dict>(
      result.attr("__dict__"))().update(copyable.attr("__dict__"));
    return result;
  }

  template<class Copyable>
  boost::python::object MakeDeepCopy(boost::python::object copyable,
      boost::python::dict memo) {
    boost::python::object copyMod = boost::python::import("copy");
    boost::python::object deepcopy = copyMod.attr("deepcopy");
    auto newCopyable = new Copyable{boost::python::extract<const Copyable&>(
      copyable)};
    boost::python::object result{boost::python::detail::new_reference(
      GetManagingObject(newCopyable))};
    auto copyableId = static_cast<int>(
      reinterpret_cast<std::uintptr_t>(copyable.ptr()));
    memo[copyableId] = result;
    boost::python::extract<boost::python::dict>(
      result.attr("__dict__"))().update(deepcopy(
      boost::python::extract<boost::python::dict>(copyable.attr("__dict__"))(),
      memo));
    return result;
  }
}
}

#endif
