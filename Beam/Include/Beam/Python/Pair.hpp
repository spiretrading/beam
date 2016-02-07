#ifndef BEAM_PYTHONPAIR_HPP
#define BEAM_PYTHONPAIR_HPP
#include <utility>
#include <boost/python.hpp>
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<class T1, class T2>
  struct PairToTupleConverter {
    static PyObject* convert(const std::pair<T1, T2>& pair) {
      return boost::python::incref(boost::python::make_tuple(
        pair.first, pair.second).ptr());
    }
  };
}

  //! Exports the std::pair template.
  template<typename K, typename V>
  void ExportPair() {
    auto typeId = boost::python::type_id<std::pair<K, V>>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::to_python_converter<std::pair<K, V>,
      Details::PairToTupleConverter<K, V>>();
  }
}
}

#endif
