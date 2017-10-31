#ifndef BEAM_PYTHONVARIANT_HPP
#define BEAM_PYTHONVARIANT_HPP
#include <boost/mpl/for_each.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/static_visitor.hpp>
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Utilities/FixedString.hpp"

namespace Beam {
namespace Python {
namespace Details {
  struct VariantConversionVisitor : boost::static_visitor<PyObject*> {
    template<typename T>
    PyObject* operator()(const T& object) const {
      return boost::python::incref(boost::python::object(object).ptr());
    }
  };

  struct VariantExtractor {
    PyObject* m_object;
    bool* m_isConvertible;

    VariantExtractor(PyObject* object, bool& isConvertible)
        : m_object(object),
          m_isConvertible(&isConvertible) {}

    template<typename T>
    void operator ()(T) const {
      *m_isConvertible = *m_isConvertible ||
        boost::python::extract<T>(m_object).check();
    }
  };

  template<typename V>
  struct VariantConstructor {
    PyObject* m_object;
    boost::python::converter::rvalue_from_python_stage1_data* m_data;

    VariantConstructor(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data)
        : m_object(object),
          m_data(data) {
      m_data->convertible = nullptr;
    }

    template<typename T>
    void operator ()(T) const {
      if(m_data->convertible != nullptr) {
        return;
      }
      boost::python::extract<T> extractor(m_object);
      if(!extractor.check()) {
        return;
      }
      auto storage = reinterpret_cast<boost::python::converter::
        rvalue_from_python_storage<V>*>(m_data)->storage.bytes;
      new(storage) V(extractor());
      m_data->convertible = storage;
    }
  };

  template<typename V>
  struct VariantToPython {
    static PyObject* convert(const V& value) {
      return boost::apply_visitor(VariantConversionVisitor(), value);
    }
  };

  template<typename V>
  struct VariantFromPythonConverter {
    static void* convertible(PyObject* object) {
      if(object == Py_None) {
        return nullptr;
      }
      bool isConvertible = false;
      boost::mpl::for_each<typename V::types>(VariantExtractor(object,
        isConvertible));
      if(isConvertible) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      boost::mpl::for_each<typename V::types>(
        VariantConstructor<V>(object, data));
    }
  };
}

  //! Exports a variant to python.
  template<typename T>
  void ExportVariant() {
    auto typeId = boost::python::type_id<T>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::to_python_converter<T, Details::VariantToPython<T>>();
    boost::python::converter::registry::push_back(
      &Details::VariantFromPythonConverter<T>::convertible,
      &Details::VariantFromPythonConverter<T>::construct,
      boost::python::type_id<T>());
  }
}
}

#endif
