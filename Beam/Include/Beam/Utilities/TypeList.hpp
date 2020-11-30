#ifndef BEAM_TYPE_LIST_HPP
#define BEAM_TYPE_LIST_HPP

namespace Beam {

  /** Stores a list of types. */
  template<typename... T>
  class TypeList {};

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
}

#endif
