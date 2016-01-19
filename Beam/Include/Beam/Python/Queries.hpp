#ifndef BEAM_PYTHONQUERIES_HPP
#define BEAM_PYTHONQUERIES_HPP
#include <type_traits>
#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include "Beam/Python/Python.hpp"
#include "Beam/Queries/IndexedQuery.hpp"
#include "Beam/Queries/NativeDataType.hpp"
#include "Beam/Queries/NativeValue.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename T>
  Queries::NativeValue<T> MakeNativeValue(const typename T::Type& value) {
    return Queries::NativeValue<T>(value);
  }
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
    \param name The name to give to the IndexedQuery.
  */
  template<typename T>
  void ExportIndexedQuery(const char* name) {
    boost::python::class_<Queries::IndexedQuery<T>>(name,
      boost::python::init<>())
      .def(boost::python::init<T>())
      .add_property("index", boost::python::make_function(
        &Queries::IndexedQuery<T>::GetIndex, boost::python::return_value_policy<
        boost::python::copy_const_reference>()),
        &Queries::IndexedQuery<T>::SetIndex);
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
}
}

#endif
