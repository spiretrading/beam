#ifndef BEAM_PYTHON_UTILITIES_HPP
#define BEAM_PYTHON_UTILITIES_HPP
#include <concepts>
#include <ostream>
#include <string>
#include <type_traits>
#include <boost/lexical_cast.hpp>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include "Beam/Python/BasicTypeCaster.hpp"
#include "Beam/Utilities/DllExport.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/FixedString.hpp"
#include "Beam/Utilities/KeyValuePair.hpp"

namespace pybind11 {
  std::ostream& operator <<(std::ostream& out, const object& value);
}

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

    BEAM_EXPORT_DLL bool operator ==(const KeyValuePair& pair) const;
  };
}

namespace Beam::Python {
namespace Details {
  template<unsigned... D>
  struct to_chars {
    static constexpr char value[sizeof...(D) + 1] = {('0' + D)..., 0};
  };

  template<unsigned R, unsigned... D>
  struct explode : explode<R / 10, R % 10, D...> {};

  template<unsigned... D>
  struct explode<0, D...> : to_chars<D...> {};

  template<unsigned N>
  struct fixed_string_name : explode<N> {};
}
  /**
   * Exports the Expect class template.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void export_expect(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Expect";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<Expect<T>>(module, name.c_str())
      .def(pybind11::init())
      .def(pybind11::init<const T&>())
      .def_property_readonly("is_value", &Expect<T>::is_value)
      .def_property_readonly("is_exception", &Expect<T>::is_exception)
      .def_property_readonly("value", [] (const Expect<T>& self) -> const T& {
        return self.get();
      });
    if(!std::is_same_v<T, pybind11::object>) {
      pybind11::implicitly_convertible<Expect<T>, Expect<pybind11::object>>();
    }
  }

  /**
   * Exports default methods and operators for a pybind11 class if they exist.
   * @param class_binding The pybind11::class_ object to export methods to.
   * @return The modified class binding for chaining.
   */
  template<typename C, typename... Options>
  auto& export_default_methods(pybind11::class_<C, Options...>& class_binding) {
    using Type = C;
    using namespace pybind11;
    if constexpr(requires { Type(); }) {
      class_binding.def(pybind11::init());
    }
    if constexpr(requires { Type(std::declval<const Type&>()); }) {
      class_binding.def(pybind11::init<const Type&>());
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left == right } -> std::convertible_to<bool>; }) {
      class_binding.def(self == self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left != right } -> std::convertible_to<bool>; }) {
      class_binding.def(self != self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left < right } -> std::convertible_to<bool>; }) {
      class_binding.def(self < self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left <= right } -> std::convertible_to<bool>; }) {
      class_binding.def(self <= self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left > right } -> std::convertible_to<bool>; }) {
      class_binding.def(self > self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left >= right } -> std::convertible_to<bool>; }) {
      class_binding.def(self >= self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left + right } -> std::convertible_to<Type>; }) {
      class_binding.def(self + self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left - right } -> std::convertible_to<Type>; }) {
      class_binding.def(self - self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left * right } -> std::convertible_to<Type>; }) {
      class_binding.def(self * self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left / right } -> std::convertible_to<Type>; }) {
      class_binding.def(self / self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left % right } -> std::convertible_to<Type>; }) {
      class_binding.def(self % self);
    }
    if constexpr(requires(const Type& value) {
        { -value } -> std::convertible_to<Type>; }) {
      class_binding.def(-self);
    }
    if constexpr(requires(const Type& value) {
        { +value } -> std::convertible_to<Type>; }) {
      class_binding.def(+self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left & right } -> std::convertible_to<Type>; }) {
      class_binding.def(self & self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left | right } -> std::convertible_to<Type>; }) {
      class_binding.def(self | self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left ^ right } -> std::convertible_to<Type>; }) {
      class_binding.def(self ^ self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left << right } -> std::convertible_to<Type>; }) {
      class_binding.def(self << self);
    }
    if constexpr(requires(const Type& left, const Type& right) {
        { left >> right } -> std::convertible_to<Type>; }) {
      class_binding.def(self >> self);
    }
    if constexpr(requires(const Type& value) {
        { ~value } -> std::convertible_to<Type>; }) {
      class_binding.def(~self);
    }
    if constexpr(requires(const Type& left, int right) {
        { left + right } -> std::convertible_to<Type>; }) {
      class_binding.def(self + int());
    }
    if constexpr(requires(int left, const Type& right) {
        { left + right } -> std::convertible_to<Type>; }) {
      class_binding.def(int() + self);
    }
    if constexpr(requires(const Type& left, int right) {
        { left - right } -> std::convertible_to<Type>; }) {
      class_binding.def(self - int());
    }
    if constexpr(requires(int left, const Type& right) {
        { left - right } -> std::convertible_to<Type>; }) {
      class_binding.def(int() - self);
    }
    if constexpr(requires(const Type& left, int right) {
        { left * right } -> std::convertible_to<Type>; }) {
      class_binding.def(self * int());
    }
    if constexpr(requires(int left, const Type& right) {
        { left * right } -> std::convertible_to<Type>; }) {
      class_binding.def(int() * self);
    }
    if constexpr(requires(const Type& left, int right) {
        { left / right } -> std::convertible_to<Type>; }) {
      class_binding.def(self / int());
    }
    if constexpr(requires(int left, const Type& right) {
        { left / right } -> std::convertible_to<Type>; }) {
      class_binding.def(int() / self);
    }
    if constexpr(requires(const Type& left, float right) {
        { left + right } -> std::convertible_to<Type>; }) {
      class_binding.def(self + float());
    }
    if constexpr(requires(float left, const Type& right) {
        { left + right } -> std::convertible_to<Type>; }) {
      class_binding.def(float() + self);
    }
    if constexpr(requires(const Type& left, float right) {
        { left - right } -> std::convertible_to<Type>; }) {
      class_binding.def(self - float());
    }
    if constexpr(requires(float left, const Type& right) {
        { left - right } -> std::convertible_to<Type>; }) {
      class_binding.def(float() - self);
    }
    if constexpr(requires(const Type& left, float right) {
        { left * right } -> std::convertible_to<Type>; }) {
      class_binding.def(self * float());
    }
    if constexpr(requires(float left, const Type& right) {
        { left * right } -> std::convertible_to<Type>; }) {
      class_binding.def(float() * self);
    }
    if constexpr(requires(const Type& left, float right) {
        { left / right } -> std::convertible_to<Type>; }) {
      class_binding.def(self / float());
    }
    if constexpr(requires(float left, const Type& right) {
        { left / right } -> std::convertible_to<Type>; }) {
      class_binding.def(float() / self);
    }
    if constexpr(requires(const Type& left, double right) {
        { left + right } -> std::convertible_to<Type>; }) {
      class_binding.def(self + double());
    }
    if constexpr(requires(double left, const Type& right) {
        { left + right } -> std::convertible_to<Type>; }) {
      class_binding.def(double() + self);
    }
    if constexpr(requires(const Type& left, double right) {
        { left - right } -> std::convertible_to<Type>; }) {
      class_binding.def(self - double());
    }
    if constexpr(requires(double left, const Type& right) {
        { left - right } -> std::convertible_to<Type>; }) {
      class_binding.def(double() - self);
    }
    if constexpr(requires(const Type& left, double right) {
        { left * right } -> std::convertible_to<Type>; }) {
      class_binding.def(self * double());
    }
    if constexpr(requires(double left, const Type& right) {
        { left * right } -> std::convertible_to<Type>; }) {
      class_binding.def(double() * self);
    }
    if constexpr(requires(const Type& left, double right) {
        { left / right } -> std::convertible_to<Type>; }) {
      class_binding.def(self / double());
    }
    if constexpr(requires(double left, const Type& right) {
        { left / right } -> std::convertible_to<Type>; }) {
      class_binding.def(double() / self);
    }
    if constexpr(requires(Type& left, const Type& right) {
        { left += right } -> std::convertible_to<Type&>; }) {
      class_binding.def(self += self);
    }
    if constexpr(requires(Type& left, const Type& right) {
        { left -= right } -> std::convertible_to<Type&>; }) {
      class_binding.def(self -= self);
    }
    if constexpr(requires(Type& left, const Type& right) {
        { left *= right } -> std::convertible_to<Type&>; }) {
      class_binding.def(self *= self);
    }
    if constexpr(requires(Type& left, const Type& right) {
        { left /= right } -> std::convertible_to<Type&>; }) {
      class_binding.def(self /= self);
    }
    if constexpr(requires(Type& left, const Type& right) {
        { left %= right } -> std::convertible_to<Type&>; }) {
      class_binding.def(self %= self);
    }
    if constexpr(requires(Type& left, const Type& right) {
        { left &= right } -> std::convertible_to<Type&>; }) {
      class_binding.def(self &= self);
    }
    if constexpr(requires(Type& left, const Type& right) {
        { left |= right } -> std::convertible_to<Type&>; }) {
      class_binding.def(self |= self);
    }
    if constexpr(requires(Type& left, const Type& right) {
        { left ^= right } -> std::convertible_to<Type&>; }) {
      class_binding.def(self ^= self);
    }
    if constexpr(requires(Type& left, const Type& right) {
        { left <<= right } -> std::convertible_to<Type&>; }) {
      class_binding.def(self <<= self);
    }
    if constexpr(requires(Type& left, const Type& right) {
        { left >>= right } -> std::convertible_to<Type&>; }) {
      class_binding.def(self >>= self);
    }
    if constexpr(requires(const Type& value) {
        { value.size() } -> std::convertible_to<std::size_t>; }) {
      class_binding.def("__len__", [] (const Type& value) {
        return value.size();
      });
    } else if constexpr(requires(const Type& value) {
        { value.get_size() } -> std::convertible_to<std::size_t>; }) {
      class_binding.def("__len__", [] (const Type& value) {
        return value.get_size();
      });
    }
    if constexpr(requires(const Type& container) {
        { container.begin() };
        { container.end() }; }) {
      class_binding.def("__iter__", [] (const Type& container) {
        return make_iterator(container.begin(), container.end());
      }, keep_alive<0, 1>());
    }
    if constexpr(
        requires(const Type& container, const Type::value_type& value) {
          { container.contains(value) } -> std::convertible_to<bool>; }) {
      class_binding.def("__contains__",
        [] (const Type& container, const typename Type::value_type& value) {
          return container.contains(value);
        });
    } else if constexpr(
        requires(const Type& container, const Type::value_type& value) {
          { container.find(value) != container.end() }; }) {
      class_binding.def("__contains__",
        [] (const Type& container, const typename Type::value_type& value) {
          return container.find(value) != container.end();
        });
    }
    if constexpr(requires(const Type& container, std::size_t index) {
        { container[index] }; }) {
      class_binding.def("__getitem__",
        [] (const Type& container, std::size_t index) {
          return container[index];
        }, return_value_policy::reference_internal);
    }
    if constexpr(requires(Type& container, std::size_t index) {
        { container[index] = container[index] }; }) {
      using Value = std::remove_cvref_t<
        decltype(std::declval<Type&>()[std::declval<std::size_t>()])>;
      class_binding.def("__setitem__",
        [] (Type& container, std::size_t index, const Value& value) {
          container[index] = value;
        });
    }
    if constexpr(std::convertible_to<Type, bool>) {
      class_binding.def("__bool__", [] (const Type& self) {
        return static_cast<bool>(self);
      });
    }
    if constexpr(std::convertible_to<Type, int>) {
      class_binding.def("__int__", [] (const Type& self) {
        return static_cast<int>(self);
      });
    }
    if constexpr(std::convertible_to<Type, float>) {
      class_binding.def("__float__", [] (const Type& self) {
        return static_cast<float>(self);
      });
    } else if constexpr(std::convertible_to<Type, double>) {
      class_binding.def("__float__", [] (const Type& self) {
        return static_cast<double>(self);
      });
    }
    if constexpr(requires(std::ostream& out, const Type& value) {
        { out << value } -> std::convertible_to<std::ostream&>; }) {
      class_binding.def("__str__", &boost::lexical_cast<std::string, Type>);
    }
    if constexpr(requires(const Type& value) {
        { std::hash<Type>{}(value) } -> std::convertible_to<std::size_t>; }) {
      class_binding.def("__hash__", std::hash<Type>());
    }
    return class_binding;
  }

  /**
   * Exports the KeyValuePair class.
   * @param module The module to export to.
   */
  void export_key_value_pair(pybind11::module& module);

  /**
   * Exports utility functions and operators.
   * @param module The module to export to.
   */
  void export_utilities(pybind11::module& module);

  /**
   * Exports YAML related classes and functions.
   * @param module The module to export to.
   */
  void export_yaml(pybind11::module& module);

  /**
   * Implements a type caster for KeyValuePair types.
   * @tparam T The type of KeyValuePair to cast.
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
    static pybind11::handle cast(
      V&& value, pybind11::return_value_policy policy, pybind11::handle parent);
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
      auto& value =
        source.cast<KeyValuePair<pybind11::object, pybind11::object>&>();
      auto key_caster = KeyConverter();
      if(!key_caster.load(value.m_key, convert)) {
        return false;
      }
      auto value_caster = ValueConverter();
      if(!value_caster.load(value.m_value, convert)) {
        return false;
      }
      m_value.emplace(pybind11::detail::cast_op<typename Type::Key&&>(
        std::move(key_caster)), pybind11::detail::cast_op<
          typename Type::Value&&>(std::move(value_caster)));
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }

  /**
   * Implements a type caster for FixedStrings.
   * @tparam N The size of the FixedString to cast.
   */
  template<typename T>
  struct FixedStringTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    static constexpr auto name = pybind11::detail::_("FixedString[") +
      pybind11::detail::_(Details::fixed_string_name<Type::SIZE>::value) +
      pybind11::detail::_("]");
    static pybind11::handle cast(const Type& value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool);
    using BasicTypeCaster<T>::m_value;
  };

  template<typename T>
  pybind11::handle FixedStringTypeCaster<T>::cast(const Type& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    return pybind11::cast(value.get_data()).release();
  }

  template<typename T>
  bool FixedStringTypeCaster<T>::load(pybind11::handle source, bool) {
    if(!PyUnicode_Check(source.ptr())) {
      return false;
    }
    m_value.emplace(PyUnicode_AsUTF8(source.ptr()));
    return true;
  }
}

namespace pybind11::detail {
  template<std::size_t N>
  struct type_caster<Beam::FixedString<N>> :
    Beam::Python::FixedStringTypeCaster<Beam::FixedString<N>> {};

  template<typename K, typename V>
  struct type_caster<Beam::KeyValuePair<K, V>,
    std::enable_if_t<!std::is_same_v<K, object> &&
      !std::is_same_v<V, object>>> : Beam::Python::KeyValuePairTypeCaster<
        Beam::KeyValuePair<K, V>> {};
}

#endif
