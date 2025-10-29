#ifndef BEAM_PYTHON_COLLECTIONS_HPP
#define BEAM_PYTHON_COLLECTIONS_HPP
#include <string_view>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include "Beam/Collections/Enum.hpp"
#include "Beam/Collections/EnumSet.hpp"
#include "Beam/Collections/View.hpp"
#include "Beam/Python/BasicTypeCaster.hpp"
#include "Beam/Python/Utilities.hpp"

namespace Beam::Python {

  /** Exports the contents of Collections. */
  void export_collections(pybind11::module& module);

  /**
   * Exports a generic EnumSet.
   * @param module The module to export to.
   * @param name The name of the type to export.
   */
  template<typename T>
  void export_enum_set(pybind11::module& module, std::string_view name) {
    auto enum_set = pybind11::class_<T>(module, name.data()).
      def(pybind11::init<typename T::Type>()).
      def(pybind11::init<typename T::Type::Type>()).
      def("test", &T::test).
      def("set", &T::set).
      def("reset", &T::reset);
    export_default_methods(enum_set);
  }

  /**
   * Exports the View template.
   * @param module The module to export to.
   * @param name The name of the class to export.
   */
  template<typename T>
  void export_view(pybind11::module& module, std::string_view name) {
    auto view = pybind11::class_<View<T>>(module, name.data()).
      def_property_readonly("size", &View<T>::size).
      def_property_readonly("empty", &View<T>::empty).
      def(
        "front", pybind11::overload_cast<>(&View<T>::front, pybind11::const_)).
      def("back", pybind11::overload_cast<>(&View<T>::back, pybind11::const_));
    export_default_methods(view);
    module.def(
      "drop_last", pybind11::overload_cast<const View<T>&>(&drop_last<T>));
  }

  /**
   * Implements a type caster for Beam::Enum types.
   * @tparam T The type of enum to cast.
   */
  template<typename T>
  struct EnumTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    using Converter = pybind11::detail::make_caster<typename Type::Type>;
    static constexpr auto name = pybind11::detail::_("Enum[") +
      Converter::name + pybind11::detail::_("]");
    static pybind11::handle cast(Type value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool convert);
    using BasicTypeCaster<T>::m_value;
  };

  template<typename T>
  pybind11::handle EnumTypeCaster<T>::cast(Type value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    policy = pybind11::detail::return_value_policy_override<
      typename Type::Type>::policy(policy);
    return Converter::cast(
      static_cast<typename Type::Type>(value), policy, parent);
  }

  template<typename T>
  bool EnumTypeCaster<T>::load(pybind11::handle source, bool convert) {
    auto caster = Converter();
    if(!caster.load(source, convert)) {
      return false;
    }
    m_value.emplace(
      pybind11::detail::cast_op<typename Type::Type&&>(std::move(caster)));
    return true;
  }
}

namespace pybind11::detail {
  template<typename T, std::size_t N>
  struct type_caster<Beam::Enum<T, N>> : Beam::Python::EnumTypeCaster<
    Beam::Enum<T, N>> {};
}

#endif
