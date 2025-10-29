#ifndef BEAM_DEREFERENCE_HPP
#define BEAM_DEREFERENCE_HPP
#include <memory>
#include <type_traits>
#include <utility>

namespace Beam {

  /** Tests if a type can be dereferenced. */
  template<typename T>
  concept IsDereferenceable =
    !std::is_class_v<std::remove_cvref_t<T>> && requires(T& t) { *t; } ||
    std::is_class_v<std::remove_cvref_t<T>> && requires(T & t) {
      *t;
      t.operator->();
    };

  template<typename T>
  inline constexpr bool is_managed_pointer_v = false;

  template<typename T, typename D>
  inline constexpr bool is_managed_pointer_v<std::unique_ptr<T, D>> = true;

  template<typename T>
  inline constexpr bool is_managed_pointer_v<std::shared_ptr<T>> = true;

  /** Tests if a type represents a 'managed' pointer (unique_ptr/shared_ptr). */
  template<typename T>
  concept IsManagedPointer = is_managed_pointer_v<std::remove_cvref_t<T>>;

  /**
   * If T can be dereferenced, yields the dereferenced type
   * (with references removed). Otherwise yields T.
   */
  template<typename T>
  struct dereference {
    using type = std::remove_cvref_t<T>;
  };

  template<IsDereferenceable T>
  struct dereference<T> {
    using type = std::remove_cvref_t<decltype(*std::declval<T&>())>;
  };

  template<typename T>
  using dereference_t = typename dereference<T>::type;

  template<typename T>
  decltype(auto) fully_dereference(T&& value) {
    if constexpr(IsDereferenceable<T>) {
      return fully_dereference(*value);
    } else {
      return std::forward<T>(value);
    }
  }
}

#endif
