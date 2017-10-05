#ifndef BEAM_PYTHON_TUPLE_HPP
#define BEAM_PYTHON_TUPLE_HPP
#include <boost/python.hpp>
#include "Beam/Python/Python.hpp"
#include "Beam/Utilities/ApplyTuple.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename... T>
  struct TupleToPythonConverter {
    static PyObject* convert(const std::tuple<T...>& t) {
      return boost::python::incref(Apply(t,
        [] (auto&&... args) {
          return boost::python::make_tuple(
            std::forward<decltype(args)>(args)...);
        }).ptr());
    }
  };
}

  //! Exports the std::tuple template.
  template<typename... T>
  void ExportTuple() {
    auto typeId = boost::python::type_id<std::tuple<T...>>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::to_python_converter<std::tuple<T...>,
      Details::TupleToPythonConverter<T...>>();
  }
}
}

#endif
