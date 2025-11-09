#ifndef BEAM_TYPE_TRAITS_HPP
#define BEAM_TYPE_TRAITS_HPP
#include <concepts>
#include <type_traits>

namespace Beam {

  /**
   * Type trait that disables constructors when an argument is the same type as
   * the target type.
   */
  template<typename T, typename... Args>
  struct disable_copy_constructor {
    using type = T;
  };

  template<typename T>
  struct disable_copy_constructor<T, T> {};

  template<typename T>
  struct disable_copy_constructor<T, T&> {};

  template<typename T>
  struct disable_copy_constructor<T, const T&> {};

  template<typename T>
  struct disable_copy_constructor<T, T&&> {};

  template<typename T>
  struct disable_copy_constructor<T, const T&&> {};

  template<typename T, typename... Args>
  using disable_copy_constructor_t =
    typename disable_copy_constructor<T, Args...>::type;

  /**
   * Disables constructors when an argument is the same type as the
   * target type.
   * @tparam T The target type.
   */
  template<typename T, typename U>
  concept DisableCopy = requires {
    typename disable_copy_constructor_t<U, T>;
  };

  /**
   * Returns a type's inverse.
   * @tparam T The type whose inverse is provided.
   */
  template<typename T>
  struct inverse {};

  template<typename T>
  using inverse_t = typename inverse<T>::type;

  /**
   * Checks if a type is an instantiation of a given template.
   * @tparam R The template that is expected to be instantiated.
   * @tparam T The type to test.
   */
  template<template<typename...> class R, typename T>
  struct is_instance : std::false_type {};

  template<template<typename...> class R, typename... Args>
  struct is_instance<R, R<Args...>> : std::true_type {};

  template<template<typename...> class R, typename T>
  struct is_instance<R, const T> : is_instance<R, T> {};

  template<template<typename...> class R, typename T>
  struct is_instance<R, volatile T> : is_instance<R, T> {};

  template<template<typename...> class R, typename T>
  struct is_instance<R, const volatile T> : is_instance<R, T> {};

  /**
   * Checks if a type is an instantiation of a given template.
   * @tparam T The type to test.
   * @tparam R The template that is expected to be instantiated.
   */
  template<typename T, template<typename...> class R>
  inline constexpr auto is_instance_v = is_instance<R, T>::value;

  /**
   * Checks if a type is an instantiation of a given template.
   * @tparam T The type to test.
   * @tparam R The template that is expected to be instantiated.
   */
  template<typename T, template<typename...> class R>
  concept IsInstance = is_instance_v<T, R>;

  /**
   * Is satisfied if a type is a subclass of some generic base class.
   * @tparam T The type to test.
   * @tparam B The generic base class that <i>T</i> is a subclass of.
   */
  template<typename T, template <typename...> class B>
  concept IsSubclass = requires(T* t) {
    [] <typename... Args>(B<Args...>*){}(t);
  };

  /**
   * Checks if a callable can be invoked with a type that has cv-ref
   * qualifications applied from another type.
   * @tparam F The callable type to check.
   * @tparam Self The type whose cv-ref qualifications are applied.
   * @tparam T The type to which the qualifications are applied.
   */
  template<typename F, typename Self, typename T>
  concept IsInvocableLike = std::invocable<
    F, std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>,
      const T&, std::conditional_t<std::is_lvalue_reference_v<Self>, T&, T&&>>>;

  template<typename T, typename V>
  concept IsConstructibleTo = std::constructible_from<V, const T&> ||
    std::constructible_from<V, T&&>;
}

#endif
