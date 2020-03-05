#ifndef BEAM_PYTHON_QUERIES_HPP
#define BEAM_PYTHON_QUERIES_HPP
#include <string>
#include <type_traits>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include "Beam/Python/BasicTypeCaster.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/IndexedQuery.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/NativeDataType.hpp"
#include "Beam/Queries/NativeValue.hpp"
#include "Beam/Queries/SequencedValue.hpp"

namespace Beam::Python {

  /**
   * Exports the BasicQuery class.
   * @param module The module to export to.
   */
  void ExportBasicQuery(pybind11::module& module);

  /**
   * Exports the ConstantExpression class.
   * @param module The module to export to.
   */
  void ExportConstantExpression(pybind11::module& module);

  /**
   * Exports the DataType class.
   * @param module The module to export to.
   */
  void ExportDataType(pybind11::module& module);

  /**
   * Exports the Expression class.
   * @param module The module to export to.
   */
  void ExportExpression(pybind11::module& module);

  /**
   * Exports the FilteredQuery class.
   * @param module The module to export to.
   */
  void ExportFilteredQuery(pybind11::module& module);

  /**
   * Exports the FunctionExpression class.
   * @param module The module to export to.
   */
  void ExportFunctionExpression(pybind11::module& module);

  /**
   * Exports the IndexListQuery class.
   * @param module The module to export to.
   */
  void ExportIndexListQuery(pybind11::module& module);

  /**
   * Exports the IndexedQuery class.
   * @param module The module to export to.
   */
  void ExportIndexedQuery(pybind11::module& module);

  /**
   * Exports the IndexedValue class.
   * @param module The module to export to.
   */
  void ExportIndexedValue(pybind11::module& module);

  /**
   * Exports the InterruptableQuery class.
   * @param module The module to export to.
   */
  void ExportInterruptableQuery(pybind11::module& module);

  /**
   * Exports the InterruptionPolicy enum.
   * @param module The module to export to.
   */
  void ExportInterruptionPolicy(pybind11::module& module);

  /**
   * Exports the MemberAccessExpression class.
   * @param module The module to export to.
   */
  void ExportMemberAccessExpression(pybind11::module& module);

  /**
   * Exports the ParameterExpression class.
   * @param module The module to export to.
   */
  void ExportParameterExpression(pybind11::module& module);

  /**
   * Exports the Queries namespace.
   * @param module The module to export to.
   */
  void ExportQueries(pybind11::module& module);

  /**
   * Exports the Range class.
   * @param module The module to export to.
   */
  void ExportRange(pybind11::module& module);

  /**
   * Exports the RangedQuery class.
   * @param module The module to export to.
   */
  void ExportRangedQuery(pybind11::module& module);

  /**
   * Exports the Sequence class.
   * @param module The module to export to.
   */
  void ExportSequence(pybind11::module& module);

  /**
   * Exports the SequencedValue class.
   * @param module The module to export to.
   */
  void ExportSequencedValue(pybind11::module& module);

  /**
   * Exports the SnapshotLimit class.
   * @param module The module to export to.
   */
  void ExportSnapshotLimit(pybind11::module& module);

  /**
   * Exports the SnapshotLimitedQuery class.
   * @param module The module to export to.
   */
  void ExportSnapshotLimitedQuery(pybind11::module& module);

  /**
   * Exports the Value class.
   * @param module The module to export to.
   */
  void ExportValue(pybind11::module& module);

  /**
   * Exports the generic NativeDataType class.
   * @param module The module to export to.
   * @param name The name of the type to export.
   */
  template<typename T>
  void ExportNativeDataType(pybind11::module& module, const std::string& name) {
    pybind11::class_<T, Queries::VirtualDataType>(module, name.c_str())
      .def(pybind11::init());
    pybind11::implicitly_convertible<T, Queries::DataType>();
  }

  /**
   * Exports the generic NativeValue class.
   * @param module The module to export to.
   * @param name The name of the type to export.
   */
  template<typename T>
  void ExportNativeValue(pybind11::module& module, const std::string& name) {
    pybind11::class_<T, Queries::VirtualValue>(module, name.c_str())
      .def(pybind11::init())
      .def(pybind11::init<typename T::Type::Type>())
      .def(pybind11::self == pybind11::self)
      .def(pybind11::self != pybind11::self);
    pybind11::implicitly_convertible<T, Queries::Value>();
    module.def("make_value",
      &Queries::MakeNativeValue<const typename T::Type::Type&>);
  }

  /**
   * Implements a type caster for BasicQuery types.
   * @param <T> The type of BasicQuery to cast.
   */
  template<typename T>
  struct BasicQueryTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    using IndexConverter = pybind11::detail::make_caster<typename Type::Index>;
    static constexpr auto name = pybind11::detail::_("BasicQuery[") +
      IndexConverter::name + pybind11::detail::_("]");
    template<typename V>
    static pybind11::handle cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool convert);
    using BasicTypeCaster<T>::m_value;
  };

  /**
   * Implements a type caster for IndexedQuery types.
   * @param <T> The type of IndexedQuery to cast.
   */
  template<typename T>
  struct IndexedQueryTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    using IndexConverter = pybind11::detail::make_caster<typename Type::Index>;
    static constexpr auto name = pybind11::detail::_("IndexedQuery[") +
      IndexConverter::name + pybind11::detail::_("]");
    template<typename V>
    static pybind11::handle cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool convert);
    using BasicTypeCaster<T>::m_value;
  };

  /**
   * Implements a type caster for IndexedValue types.
   * @param <T> The type of IndexedValue to cast.
   */
  template<typename T>
  struct IndexedValueTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    using IndexConverter = pybind11::detail::make_caster<typename Type::Index>;
    using ValueConverter = pybind11::detail::make_caster<typename Type::Value>;
    static constexpr auto name = pybind11::detail::_("IndexedValue[") +
      IndexConverter::name + pybind11::detail::_(",") +
      ValueConverter::name + pybind11::detail::_("]");
    template<typename V>
    static pybind11::handle cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool convert);
    using BasicTypeCaster<T>::m_value;
  };

  /**
   * Implements a type caster for SequencedValue types.
   * @param <T> The type of SequencedValue to cast.
   */
  template<typename T>
  struct SequencedValueTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    using ValueConverter = pybind11::detail::make_caster<typename Type::Value>;
    static constexpr auto name = pybind11::detail::_("SequencedValue[") +
      ValueConverter::name + pybind11::detail::_("]");
    template<typename V>
    static pybind11::handle cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool convert);
    using BasicTypeCaster<T>::m_value;
  };

  template<typename T>
  template<typename V>
  pybind11::handle BasicQueryTypeCaster<T>::cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    policy = pybind11::detail::return_value_policy_override<
      typename Type::Index>::policy(policy);
    auto query = Queries::BasicQuery<pybind11::object>();
    query.SetIndex(pybind11::reinterpret_steal<pybind11::object>(
      IndexConverter::cast(std::forward<V>(value).GetIndex(), policy, parent)));
    query.SetRange(std::forward<V>(value).GetRange());
    query.SetSnapshotLimit(std::forward<V>(value).GetSnapshotLimit());
    query.SetInterruptionPolicy(value.GetInterruptionPolicy());
    query.SetFilter(std::forward<V>(value).GetFilter());
    return pybind11::detail::make_caster<
      Queries::BasicQuery<pybind11::object>>::cast(std::move(query), policy,
      parent);
  }

  template<typename T>
  bool BasicQueryTypeCaster<T>::load(pybind11::handle source, bool convert) {
    try {
      auto& query = source.cast<Queries::BasicQuery<pybind11::object>&>();
      auto indexCaster = IndexConverter();
      if(!indexCaster.load(query.GetIndex(), convert)) {
        return false;
      }
      m_value.emplace();
      m_value->SetIndex(pybind11::detail::cast_op<typename Type::Index&&>(
        std::move(indexCaster)));
      m_value->SetRange(query.GetRange());
      m_value->SetSnapshotLimit(query.GetSnapshotLimit());
      m_value->SetInterruptionPolicy(query.GetInterruptionPolicy());
      m_value->SetFilter(query.GetFilter());
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }

  template<typename T>
  template<typename V>
  pybind11::handle IndexedQueryTypeCaster<T>::cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    policy = pybind11::detail::return_value_policy_override<
      typename Type::Index>::policy(policy);
    return pybind11::cast(Queries::IndexedQuery(
      pybind11::reinterpret_steal<pybind11::object>(
      IndexConverter::cast(std::forward<V>(value).GetIndex(), policy,
      parent)))).release();
  }

  template<typename T>
  bool IndexedQueryTypeCaster<T>::load(pybind11::handle source, bool convert) {
    try {
      auto& query = source.cast<Queries::IndexedQuery<pybind11::object>&>();
      auto indexCaster = IndexConverter();
      if(!indexCaster.load(query.GetIndex(), convert)) {
        return false;
      }
      m_value.emplace(pybind11::detail::cast_op<typename Type::Index&&>(
        std::move(indexCaster)));
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }

  template<typename T>
  template<typename V>
  pybind11::handle IndexedValueTypeCaster<T>::cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    auto valuePolicy = pybind11::detail::return_value_policy_override<
      typename Type::Value>::policy(policy);
    auto object = pybind11::reinterpret_steal<pybind11::object>(
      ValueConverter::cast(std::forward<V>(value).GetValue(), valuePolicy,
      parent));
    auto indexPolicy = pybind11::detail::return_value_policy_override<
      typename Type::Index>::policy(policy);
    auto index = pybind11::reinterpret_steal<pybind11::object>(
      IndexConverter::cast(std::forward<V>(value).GetIndex(), indexPolicy,
      parent));
    return pybind11::cast(Queries::IndexedValue(std::move(object),
      std::move(index))).release();
  }

  template<typename T>
  bool IndexedValueTypeCaster<T>::load(pybind11::handle source, bool convert) {
    try {
      auto& indexedValue = source.cast<
        Queries::IndexedValue<pybind11::object, pybind11::object>&>();
      auto indexCaster = IndexConverter();
      if(!indexCaster.load(indexedValue.GetIndex(), convert)) {
        return false;
      }
      auto valueCaster = ValueConverter();
      if(!valueCaster.load(indexedValue.GetValue(), convert)) {
        return false;
      }
      m_value.emplace(pybind11::detail::cast_op<typename Type::Value&&>(
        std::move(valueCaster)),
        pybind11::detail::cast_op<typename Type::Index&&>(
        std::move(indexCaster)));
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }

  template<typename T>
  template<typename V>
  pybind11::handle SequencedValueTypeCaster<T>::cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    auto valuePolicy = pybind11::detail::return_value_policy_override<
      typename Type::Value>::policy(policy);
    auto object = pybind11::reinterpret_steal<pybind11::object>(
      ValueConverter::cast(std::forward<V>(value).GetValue(), valuePolicy,
      parent));
    return pybind11::cast(Queries::SequencedValue(std::move(object),
      std::forward<V>(value).GetSequence())).release();
  }

  template<typename T>
  bool SequencedValueTypeCaster<T>::load(pybind11::handle source,
      bool convert) {
    try {
      auto& sequencedValue = source.cast<
        Queries::SequencedValue<pybind11::object>&>();
      auto valueCaster = ValueConverter();
      if(!valueCaster.load(sequencedValue.GetValue(), convert)) {
        return false;
      }
      m_value.emplace(pybind11::detail::cast_op<typename Type::Value&&>(
        std::move(valueCaster)), sequencedValue.GetSequence());
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }
}

namespace pybind11::detail {
  template<typename T>
  struct type_caster<Beam::Queries::BasicQuery<T>,
    std::enable_if_t<!std::is_same_v<T, object>>> :
    Beam::Python::BasicQueryTypeCaster<Beam::Queries::BasicQuery<T>> {};

  template<typename T>
  struct type_caster<Beam::Queries::IndexedQuery<T>,
    std::enable_if_t<!std::is_same_v<T, object>>> :
    Beam::Python::IndexedQueryTypeCaster<Beam::Queries::IndexedQuery<T>> {};

  template<typename V, typename I>
  struct type_caster<Beam::Queries::IndexedValue<V, I>,
    std::enable_if_t<!std::is_same_v<V, object> &&
    !std::is_same_v<I, object>>> : Beam::Python::IndexedValueTypeCaster<
    Beam::Queries::IndexedValue<V, I>> {};

  template<typename T>
  struct type_caster<Beam::Queries::SequencedValue<T>,
    std::enable_if_t<!std::is_same_v<T, object>>> :
    Beam::Python::SequencedValueTypeCaster<Beam::Queries::SequencedValue<T>> {};
}

#endif
