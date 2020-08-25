#ifndef BEAM_PYTHON_KEY_VALUE_PAIR_HPP
#define BEAM_PYTHON_KEY_VALUE_PAIR_HPP
#include "Beam/Python/BasicTypeCaster.hpp"
#include "Beam/Utilities/DllExport.hpp"
#include "Beam/Utilities/KeyValuePair.hpp"

namespace Beam {

  /**
   * Specialization of KeyValuePair when the key and value are
   * pybind11::objects.
   */
  template<>
  struct KeyValuePair<pybind11::object, pybind11::object> {
    using Key = pybind11::object;
    using Value = pybind11::object;

    Key m_key;
    Value m_value;

    KeyValuePair() = default;
    BEAM_EXPORT_DLL KeyValuePair(Key key, Value value);
    BEAM_EXPORT_DLL bool operator ==(const KeyValuePair& pair) const;
    BEAM_EXPORT_DLL bool operator !=(const KeyValuePair& pair) const;
  };
}

namespace Beam::Python {

  /**
   * Exports the KeyValuePair class.
   * @param module The module to export to.
   */
  void ExportKeyValuePair(pybind11::module& module);

  /**
   * Implements a type caster for KeyValuePair types.
   * @param <T> The type of KeyValuePair to cast.
   */
  template<typename T>
  struct KeyValuePairTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    using KeyConverter = pybind11::detail::make_caster<typename Type::Key>;
    using ValueConverter = pybind11::detail::make_caster<typename Type::Value>;
    static constexpr auto name = pybind11::detail::_("KeyValuePair[") +
      KeyConverter::name + pybind11::detail::_(",") + ValueConverter::name +
      pybind11::detail::_("]");
    template<typename V>
    static pybind11::handle cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool convert);
    using BasicTypeCaster<T>::m_value;
  };

  template<typename T>
  template<typename V>
  pybind11::handle KeyValuePairTypeCaster<T>::cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    auto pair = KeyValuePair<pybind11::object, pybind11::object>(
      pybind11::reinterpret_steal<pybind11::object>(
      KeyConverter::cast(std::forward<V>(value).m_key,
      pybind11::detail::return_value_policy_override<
      typename Type::Key>::policy(policy), parent)),
      pybind11::reinterpret_steal<pybind11::object>(
      ValueConverter::cast(std::forward<V>(value).m_value,
      pybind11::detail::return_value_policy_override<
      typename Type::Value>::policy(policy), parent)));
    return pybind11::detail::make_caster<KeyValuePair<
      pybind11::object, pybind11::object>>::cast(std::move(pair), policy,
      parent);
  }

  template<typename T>
  bool KeyValuePairTypeCaster<T>::load(pybind11::handle source, bool convert) {
    try {
      auto& value = source.cast<
        KeyValuePair<pybind11::object, pybind11::object>&>();
      auto keyCaster = KeyConverter();
      if(!keyCaster.load(value.m_key, convert)) {
        return false;
      }
      auto valueCaster = ValueConverter();
      if(!valueCaster.load(value.m_value, convert)) {
        return false;
      }
      m_value.emplace(pybind11::detail::cast_op<typename Type::Key&&>(
        std::move(keyCaster)), pybind11::detail::cast_op<
        typename Type::Value&&>(std::move(valueCaster)));
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }
}

namespace pybind11::detail {
  template<typename K, typename V>
  struct type_caster<Beam::KeyValuePair<K, V>,
    std::enable_if_t<!std::is_same_v<K, object> &&
    !std::is_same_v<V, object>>> : Beam::Python::KeyValuePairTypeCaster<
    Beam::KeyValuePair<K, V>> {};
}

#endif
