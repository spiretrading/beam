#ifndef BEAM_PYTHONQUERIES_HPP
#define BEAM_PYTHONQUERIES_HPP
#include <type_traits>
#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include "Beam/Python/Copy.hpp"
#include "Beam/Python/Python.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/IndexedQuery.hpp"
#include "Beam/Queries/NativeDataType.hpp"
#include "Beam/Queries/NativeValue.hpp"
#include "Beam/Queries/SequencedValue.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename T>
  Queries::NativeValue<T> MakeNativeValue(const typename T::Type& value) {
    return Queries::NativeValue<T>(value);
  }

  template<typename T>
  struct IndexedQueryToPython {
    static PyObject* convert(const Queries::IndexedQuery<T>& query) {
      Queries::IndexedQuery<boost::python::object> result{
        boost::python::object{query.GetIndex()}};
      return boost::python::incref(boost::python::object{result}.ptr());
    }
  };

  template<typename T>
  struct IndexedQueryFromPythonConverter {
    static void* convertible(PyObject* object) {
      boost::python::extract<Queries::IndexedQuery<boost::python::object>>
        queryExtractor{object};
      if(queryExtractor.check()) {
        if(boost::python::extract<T>{queryExtractor().GetIndex()}.check()) {
          return object;
        }
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      auto query = boost::python::extract<
        Queries::IndexedQuery<boost::python::object>>{object}();
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<
        Queries::IndexedQuery<T>>*>(data)->storage.bytes;
      new(storage) Queries::IndexedQuery<T>{boost::python::extract<T>{
        query.GetIndex()}()};
      data->convertible = storage;
    }
  };

  template<typename T>
  struct BasicQueryToPython {
    static PyObject* convert(const Queries::BasicQuery<T>& query) {
      Queries::BasicQuery<boost::python::object> result;
      result.SetIndex(boost::python::object{query.GetIndex()});
      result.SetRange(query.GetRange());
      result.SetSnapshotLimit(query.GetSnapshotLimit());
      result.SetInterruptionPolicy(query.GetInterruptionPolicy());
      result.SetFilter(query.GetFilter());
      return boost::python::incref(boost::python::object{result}.ptr());
    }
  };

  template<typename T>
  struct BasicQueryFromPythonConverter {
    static void* convertible(PyObject* object) {
      boost::python::extract<Queries::BasicQuery<boost::python::object>>
        queryExtractor{object};
      if(queryExtractor.check()) {
        if(boost::python::extract<T>{queryExtractor().GetIndex()}.check()) {
          return object;
        }
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      auto query = boost::python::extract<
        Queries::BasicQuery<boost::python::object>>{object}();
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<
        Queries::BasicQuery<T>>*>(data)->storage.bytes;
      new(storage) Queries::BasicQuery<T>{};
      auto result = reinterpret_cast<Queries::BasicQuery<T>*>(storage);
      result->SetIndex(boost::python::extract<T>{query.GetIndex()}());
      result->SetRange(query.GetRange());
      result->SetSnapshotLimit(query.GetSnapshotLimit());
      result->SetInterruptionPolicy(query.GetInterruptionPolicy());
      result->SetFilter(query.GetFilter());
      data->convertible = storage;
    }
  };
}

  //! Exports the ConstantExpression class.
  void ExportConstantExpression();

  //! Exports the DataType class.
  void ExportDataType();

  //! Exports the Expression class.
  void ExportExpression();

  //! Exports the FilteredQuery class.
  void ExportFilteredQuery();

  //! Exports the FunctionExpression class.
  void ExportFunctionExpression();

  //! Exports the InterruptableQuery class.
  void ExportInterruptableQuery();

  //! Exports the InterruptionPolicy enum.
  void ExportInterruptionPolicy();

  //! Exports the MemberAccessExpression class.
  void ExportMemberAccessExpression();

  //! Exports the ParameterExpression class.
  void ExportParameterExpression();

  //! Exports the Queries namespace.
  void ExportQueries();

  //! Exports the Range class.
  void ExportRange();

  //! Exports the RangedQuery class.
  void ExportRangedQuery();

  //! Exports the Sequence class.
  void ExportSequence();

  //! Exports the SnapshotLimit class.
  void ExportSnapshotLimit();

  //! Exports the SnapshotLimitedQuery class.
  void ExportSnapshotLimitedQuery();

  //! Exports the Value class.
  void ExportValue();

  //! Exports an IndexedQuery.
  /*!
    \param indexName The name of the index.
  */
  template<typename Index>
  void ExportIndexedQuery(const char* name) {
    auto typeId = boost::python::type_id<Queries::IndexedQuery<Index>>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    if(std::is_same<Index, boost::python::object>::value) {
      boost::python::class_<Queries::IndexedQuery<Index>>(
        (name + std::string{"IndexedQuery"}).c_str(), boost::python::init<>())
        .def(boost::python::init<Index>())
        .def("__copy__", &MakeCopy<Queries::IndexedQuery<Index>>)
        .def("__deepcopy__", &MakeDeepCopy<Queries::IndexedQuery<Index>>)
        .add_property("index", boost::python::make_function(
          &Queries::IndexedQuery<Index>::GetIndex,
          boost::python::return_value_policy<
          boost::python::copy_const_reference>()),
          &Queries::IndexedQuery<Index>::SetIndex);
    } else {
      boost::python::to_python_converter<Queries::IndexedQuery<Index>,
        Details::IndexedQueryToPython<Index>>();
      boost::python::converter::registry::push_back(
        &Details::IndexedQueryFromPythonConverter<Index>::convertible,
        &Details::IndexedQueryFromPythonConverter<Index>::construct,
        boost::python::type_id<Queries::IndexedQuery<Index>>());
    }
  }

  //! Exports the BasicQuery class.
  /*!
    \param indexName The name of the index.
  */
  template<typename Index>
  void ExportBasicQuery(const char* name) {
    auto typeId = boost::python::type_id<Queries::BasicQuery<Index>>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    ExportIndexedQuery<Index>(name);
    if(std::is_same<Index, boost::python::object>::value) {
      boost::python::class_<Queries::BasicQuery<Index>,
        boost::python::bases<Queries::IndexedQuery<Index>, Queries::RangedQuery,
        Queries::SnapshotLimitedQuery, Queries::InterruptableQuery,
        Queries::FilteredQuery>>((name + std::string{"Query"}).c_str(),
        boost::python::init<>())
        .def("__copy__", &MakeCopy<Queries::BasicQuery<Index>>)
        .def("__deepcopy__", &MakeDeepCopy<Queries::BasicQuery<Index>>);
    } else {
      boost::python::to_python_converter<Queries::BasicQuery<Index>,
        Details::BasicQueryToPython<Index>>();
      boost::python::converter::registry::push_back(
        &Details::BasicQueryFromPythonConverter<Index>::convertible,
        &Details::BasicQueryFromPythonConverter<Index>::construct,
        boost::python::type_id<Queries::BasicQuery<Index>>());
    }
  }

  //! Exports a NativeDataType.
  /*!
    \param name The name to give to the NativeDataType.
  */
  template<typename T>
  void ExportNativeDataType(const char* name) {
    boost::python::class_<T, boost::python::bases<Queries::VirtualDataType>>(
      name, boost::python::init<>());
    boost::python::implicitly_convertible<T, ClonePtr<T>>();
    boost::python::implicitly_convertible<T, Queries::DataType>();
  }

  //! Exports a NativeValue.
  /*!
    \param name The name to give to the NativeValue.
  */
  template<typename T>
  void ExportNativeValue(const char* name) {
    boost::python::class_<T, boost::python::bases<Queries::VirtualValue>>(name,
      boost::python::init<>())
      .def(boost::python::init<typename T::Type::Type>())
      .def(boost::python::self == boost::python::self)
      .def(boost::python::self != boost::python::self);
    boost::python::implicitly_convertible<T, ClonePtr<T>>();
    boost::python::implicitly_convertible<T, Queries::Value>();
    boost::python::def("make_value",
      &Details::MakeNativeValue<typename T::Type>);
  }

  //! Exports a SequencedValue.
  /*!
    \param name The name to give to the SequencedValue.
  */
  template<typename T>
  void ExportSequencedValue(const char* name) {
    boost::python::class_<Queries::SequencedValue<T>>(name,
        boost::python::init<>())
      .def(boost::python::init<const T&, Queries::Sequence>())
      .add_property("value", boost::python::make_function(
        static_cast<const T& (Queries::SequencedValue<T>::*)() const>(
        &Queries::SequencedValue<T>::GetValue),
        boost::python::return_value_policy<
        boost::python::copy_const_reference>()))
      .add_property("sequence", boost::python::make_function(
        static_cast<Queries::Sequence (Queries::SequencedValue<T>::*)() const>(
        &Queries::SequencedValue<T>::GetSequence)))
      .def(boost::python::self == boost::python::self)
      .def(boost::python::self != boost::python::self);
  }
}
}

#endif
