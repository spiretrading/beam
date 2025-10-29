#ifndef BEAM_PYTHON_QUERIES_HPP
#define BEAM_PYTHON_QUERIES_HPP
#include <string>
#include <string_view>
#include <type_traits>
#include <boost/lexical_cast.hpp>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include "Beam/Python/BasicTypeCaster.hpp"
#include "Beam/Utilities/DllExport.hpp"
#include "Beam/Python/Optional.hpp"
#include "Beam/Python/Utilities.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/IndexedQuery.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/PagedQuery.hpp"
#include "Beam/Queries/SequencedValue.hpp"

namespace Beam::Python {

  /** Returns the exported Value class. */
  BEAM_EXPORT_DLL pybind11::class_<Value>& get_exported_value();

  /** Exports the AndExpression class. */
  void export_and_expression(pybind11::module& module);

  /** Exports a generic BasicQuery class. */
  template<typename T>
  void export_basic_query(pybind11::module& module, std::string_view name) {
    auto query = pybind11::class_<BasicQuery<T>, IndexedQuery<T>, RangedQuery,
      SnapshotLimitedQuery, InterruptableQuery, FilteredQuery>(
        module, name.data());
    export_default_methods(query);
  }

  /** Exports the ConstantExpression class. */
  void export_constant_expression(pybind11::module& module);

  /** Exports the Expression class. */
  void export_expression(pybind11::module& module);

  /** Exports the ExpressionQuery class. */
  void export_expression_query(pybind11::module& module);

  /** Exports the FilteredQuery class. */
  void export_filtered_query(pybind11::module& module);

  /** Exports the FunctionExpression class. */
  void export_function_expression(pybind11::module& module);

  /** Exports the IndexedQuery class. */
  template<typename T>
  void export_indexed_query(pybind11::module& module, std::string_view name) {
    auto query = pybind11::class_<IndexedQuery<T>>(module, name.data()).
      def(pybind11::init<T>()).
      def_property(
        "index", &IndexedQuery<T>::get_index, &IndexedQuery<T>::set_index);
    export_default_methods(query);
  }

  /** Exports the IndexedValue class. */
  void export_indexed_value(pybind11::module& module);

  /** Exports the InterruptableQuery class. */
  void export_interruptable_query(pybind11::module& module);

  /** Exports the InterruptionPolicy enum. */
  void export_interruption_policy(pybind11::module& module);

  /** Exports the MemberAccessExpression class. */
  void export_member_access_expression(pybind11::module& module);

  /** Exports the generic NativeValue class. */
  template<typename T>
  void export_native_value(pybind11::module& module, std::string_view name) {
    pybind11::class_<T, VirtualValue>(module, name.data()).
      def(pybind11::init<const typename T::Type&>());
    get_exported_value().def(pybind11::init<const typename T::Type&>());
    pybind11::implicitly_convertible<typename T::Type, Value>();
  }

  /** Exports the NotExpression class. */
  void export_not_expression(pybind11::module& module);

  /** Exports the OrExpression class. */
  void export_or_expression(pybind11::module& module);

  /** Exports a generic PagedQuery class. */
  template<typename I, typename A>
  void export_paged_query(pybind11::module& module, std::string_view name) {
    auto query = pybind11::class_<PagedQuery<I, A>, IndexedQuery<I>,
      SnapshotLimitedQuery, FilteredQuery>(module, name.data()).
      def_property("anchor", &PagedQuery<I, A>::get_anchor,
        pybind11::overload_cast<const boost::optional<A>&>(
          &PagedQuery<I, A>::set_anchor));
    export_default_methods(query);
  }

  /** Exports the ParameterExpression class. */
  void export_parameter_expression(pybind11::module& module);

  /** Exports the Queries namespace. */
  void export_queries(pybind11::module& module);

  /** Exports the QueryReactor. */
  void export_query_reactor(pybind11::module& module);

  /** Exports the Range class. */
  void export_range(pybind11::module& module);

  /** Exports the RangedQuery class. */
  void export_ranged_query(pybind11::module& module);

  /** Exports the Sequence class. */
  void export_sequence(pybind11::module& module);

  /** Exports the SequencedValue class. */
  void export_sequenced_value(pybind11::module& module);

  /** Exports the SnapshotLimit class. */
  void export_snapshot_limit(pybind11::module& module);

  /** Exports the SnapshotLimitedQuery class. */
  void export_snapshot_limited_query(pybind11::module& module);

  /** Exports the Value class. */
  void export_value(pybind11::module& module);

  /** Implements a type caster for BasicQuery types. */
  template<typename T>
  struct BasicQueryTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    using IndexConverter = pybind11::detail::make_caster<typename Type::Index>;
    static constexpr auto name = pybind11::detail::_("BasicQuery[") +
      IndexConverter::name + pybind11::detail::_("]");
    template<typename V>
    static pybind11::handle cast(
      V&& value, pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool convert);
    using BasicTypeCaster<T>::m_value;
  };

  /** Implements a type caster for IndexedQuery types. */
  template<typename T>
  struct IndexedQueryTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    using IndexConverter = pybind11::detail::make_caster<typename Type::Index>;
    static constexpr auto name = pybind11::detail::_("IndexedQuery[") +
      IndexConverter::name + pybind11::detail::_("]");
    template<typename V>
    static pybind11::handle cast(
      V&& value, pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool convert);
    using BasicTypeCaster<T>::m_value;
  };

  /** Implements a type caster for IndexedValue types. */
  template<typename T>
  struct IndexedValueTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    using IndexConverter = pybind11::detail::make_caster<typename Type::Index>;
    using ValueConverter = pybind11::detail::make_caster<typename Type::Value>;
    static constexpr auto name = pybind11::detail::_("IndexedValue[") +
      IndexConverter::name + pybind11::detail::_(",") +
      ValueConverter::name + pybind11::detail::_("]");
    template<typename V>
    static pybind11::handle cast(
      V&& value, pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool convert);
    using BasicTypeCaster<T>::m_value;
  };

  /** Implements a type caster for PagedQuery types. */
  template<typename T>
  struct PagedQueryTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    using IndexConverter = pybind11::detail::make_caster<typename Type::Index>;
    using AnchorConverter = pybind11::detail::make_caster<
      typename Type::Anchor>;
    static constexpr auto name = pybind11::detail::_("PagedQuery[") +
      IndexConverter::name + pybind11::detail::_(",") + AnchorConverter::name +
      pybind11::detail::_("]");
    template<typename V>
    static pybind11::handle cast(
      V&& value, pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool convert);
    using BasicTypeCaster<T>::m_value;
  };

  /** Implements a type caster for SequencedValue types. */
  template<typename T>
  struct SequencedValueTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    using ValueConverter = pybind11::detail::make_caster<typename Type::Value>;
    static constexpr auto name = pybind11::detail::_("SequencedValue[") +
      ValueConverter::name + pybind11::detail::_("]");
    template<typename V>
    static pybind11::handle cast(
      V&& value, pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool convert);
    using BasicTypeCaster<T>::m_value;
  };

  template<typename T>
  template<typename V>
  pybind11::handle BasicQueryTypeCaster<T>::cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    policy = pybind11::detail::return_value_policy_override<
      typename Type::Index>::policy(policy);
    auto query = BasicQuery<pybind11::object>();
    query.set_index(pybind11::reinterpret_steal<pybind11::object>(
      IndexConverter::cast(std::forward<V>(value).get_index(), policy,
        parent)));
    query.set_range(std::forward<V>(value).get_range());
    query.set_snapshot_limit(std::forward<V>(value).get_snapshot_limit());
    query.set_interruption_policy(value.get_interruption_policy());
    query.set_filter(std::forward<V>(value).get_filter());
    return pybind11::detail::make_caster<BasicQuery<pybind11::object>>::cast(
      std::move(query), policy, parent);
  }

  template<typename T>
  bool BasicQueryTypeCaster<T>::load(pybind11::handle source, bool convert) {
    try {
      auto& query = source.cast<BasicQuery<pybind11::object>&>();
      auto index_caster = IndexConverter();
      if(!index_caster.load(query.get_index(), convert)) {
        return false;
      }
      m_value.emplace();
      m_value->set_index(pybind11::detail::cast_op<typename Type::Index&&>(
        std::move(index_caster)));
      m_value->set_range(query.get_range());
      m_value->set_snapshot_limit(query.get_snapshot_limit());
      m_value->set_interruption_policy(query.get_interruption_policy());
      m_value->set_filter(query.get_filter());
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }

  template<typename T>
  template<typename V>
  pybind11::handle IndexedQueryTypeCaster<T>::cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    policy =pybind11::detail::return_value_policy_override<
      typename Type::Index>::policy(policy);
    return pybind11::cast(
      IndexedQuery(pybind11::reinterpret_steal<pybind11::object>(
        IndexConverter::cast(std::forward<V>(value).get_index(), policy,
          parent)))).release();
  }

  template<typename T>
  bool IndexedQueryTypeCaster<T>::load(pybind11::handle source, bool convert) {
    try {
      auto& query = source.cast<IndexedQuery<pybind11::object>&>();
      auto index_caster = IndexConverter();
      if(!index_caster.load(query.get_index(), convert)) {
        return false;
      }
      m_value.emplace(pybind11::detail::cast_op<typename Type::Index&&>(
        std::move(index_caster)));
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }

  template<typename T>
  template<typename V>
  pybind11::handle IndexedValueTypeCaster<T>::cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    auto value_policy = pybind11::detail::return_value_policy_override<
      typename Type::Value>::policy(policy);
    auto object = pybind11::reinterpret_steal<pybind11::object>(
      ValueConverter::cast(
        std::forward<V>(value).get_value(), value_policy, parent));
    auto index_policy = pybind11::detail::return_value_policy_override<
      typename Type::Index>::policy(policy);
    auto index = pybind11::reinterpret_steal<pybind11::object>(
      IndexConverter::cast(
        std::forward<V>(value).get_index(), index_policy, parent));
    return pybind11::cast(
      IndexedValue(std::move(object), std::move(index))).release();
  }

  template<typename T>
  bool IndexedValueTypeCaster<T>::load(pybind11::handle source, bool convert) {
    try {
      auto& indexed_value =
        source.cast<IndexedValue<pybind11::object, pybind11::object>&>();
      auto index_caster = IndexConverter();
      if(!index_caster.load(indexed_value.get_index(), convert)) {
        return false;
      }
      auto value_caster = ValueConverter();
      if(!value_caster.load(indexed_value.get_value(), convert)) {
        return false;
      }
      m_value.emplace(pybind11::detail::cast_op<typename Type::Value&&>(
        std::move(value_caster)),
        pybind11::detail::cast_op<typename Type::Index&&>(
        std::move(index_caster)));
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }

  template<typename T>
  template<typename V>
  pybind11::handle PagedQueryTypeCaster<T>::cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    auto query = PagedQuery<pybind11::object, pybind11::object>();
    auto index_policy = pybind11::detail::return_value_policy_override<
      typename Type::Index>::policy(policy);
    query.set_index(pybind11::reinterpret_steal<pybind11::object>(
      IndexConverter::cast(
        std::forward<V>(value).get_index(), index_policy, parent)));
    auto anchor_policy = pybind11::detail::return_value_policy_override<
      typename Type::Anchor>::policy(policy);
    auto anchor = pybind11::reinterpret_steal<pybind11::object>(
      AnchorConverter::cast(
        std::forward<V>(value).get_anchor(), anchor_policy, parent));
    if(!anchor.is_none()) {
      query.set_anchor(std::move(anchor));
    }
    query.set_snapshot_limit(std::forward<V>(value).get_snapshot_limit());
    query.set_filter(std::forward<V>(value).get_filter());
    return pybind11::detail::make_caster<
      PagedQuery<pybind11::object, pybind11::object>>::cast(
        std::move(query), policy, parent);
  }

  template<typename T>
  bool PagedQueryTypeCaster<T>::load(pybind11::handle source, bool convert) {
    try {
      auto& query =
        source.cast<PagedQuery<pybind11::object, pybind11::object>&>();
      auto index_caster = IndexConverter();
      if(!index_caster.load(query.get_index(), convert)) {
        return false;
      }
      auto anchor_caster = AnchorConverter();
      if(query.get_anchor()) {
        if(!anchor_caster.load(*query.get_anchor(), convert)) {
          return false;
        }
      }
      m_value.emplace();
      m_value->set_index(pybind11::detail::cast_op<typename Type::Index&&>(
        std::move(index_caster)));
      if(query.get_anchor()) {
        m_value->set_anchor(pybind11::detail::cast_op<typename Type::Anchor&&>(
          std::move(anchor_caster)));
      }
      m_value->set_snapshot_limit(query.get_snapshot_limit());
      m_value->set_filter(query.get_filter());
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }

  template<typename T>
  template<typename V>
  pybind11::handle SequencedValueTypeCaster<T>::cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    auto value_policy = [&] {
      if constexpr(std::is_pointer_v<typename Type::Value>) {
        return pybind11::return_value_policy::reference;
      } else {
        return pybind11::detail::return_value_policy_override<
          typename Type::Value>::policy(policy);
      }
    }();
    auto object = pybind11::reinterpret_steal<pybind11::object>(
      ValueConverter::cast(
        std::forward<V>(value).get_value(), value_policy, parent));
    return pybind11::cast(SequencedValue(std::move(object),
      std::forward<V>(value).get_sequence())).release();
  }

  template<typename T>
  bool SequencedValueTypeCaster<T>::load(
      pybind11::handle source, bool convert) {
    try {
      auto& sequenced_value = source.cast<SequencedValue<pybind11::object>&>();
      auto value_caster = ValueConverter();
      if(!value_caster.load(sequenced_value.get_value(), convert)) {
        return false;
      }
      m_value.emplace(pybind11::detail::cast_op<typename Type::Value&&>(
        std::move(value_caster)), sequenced_value.get_sequence());
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }
}

namespace pybind11::detail {
  template<typename T>
  struct type_caster<Beam::BasicQuery<T>,
    std::enable_if_t<!std::is_same_v<T, object>>> :
      Beam::Python::BasicQueryTypeCaster<Beam::BasicQuery<T>> {};

  template<typename T>
  struct type_caster<Beam::IndexedQuery<T>,
    std::enable_if_t<!std::is_same_v<T, object>>> :
      Beam::Python::IndexedQueryTypeCaster<Beam::IndexedQuery<T>> {};

  template<typename V, typename I>
  struct type_caster<Beam::IndexedValue<V, I>,
    std::enable_if_t<!std::is_same_v<V, object> &&
      !std::is_same_v<I, object>>> :
        Beam::Python::IndexedValueTypeCaster<Beam::IndexedValue<V, I>> {};

  template<typename T, typename U>
  struct type_caster<Beam::PagedQuery<T, U>,
    std::enable_if_t<
      !std::is_same_v<T, object> && !std::is_same_v<U, object>>> :
        Beam::Python::PagedQueryTypeCaster<Beam::PagedQuery<T, U>> {};

  template<typename T>
  struct type_caster<Beam::SequencedValue<T>,
    std::enable_if_t<!std::is_same_v<T, object>>> :
      Beam::Python::SequencedValueTypeCaster<Beam::SequencedValue<T>> {};
}

#endif
