#ifndef BEAM_DEREFERENCE_HPP
#define BEAM_DEREFERENCE_HPP
#include <memory>
#include <type_traits>
#include "Beam/Pointers/Pointers.hpp"

namespace Beam {

  /** Returns <code>true</code> iff a type can be dereferenced. */
  template<typename T>
  struct IsDereferenceable {
    private:
      using YesType = char;
      using NoType = struct {
        YesType a[2];
      };

      template<typename C>
      static YesType Test(std::decay_t<
        decltype(*std::declval<C>())>* = nullptr);

      template<typename C>
      static NoType Test(...);

    public:
      static constexpr auto value =
        sizeof(Test<T>(nullptr)) == sizeof(YesType);
  };

  /** Returns <code>true</code> iff a type represents a 'managed' pointer. */
  template<typename T>
  struct IsManagedPointer {
    static constexpr auto value = false;
  };

  template<typename T, typename D>
  struct IsManagedPointer<std::unique_ptr<T, D>> {
    static constexpr auto value = true;
  };

  template<typename T>
  struct IsManagedPointer<std::shared_ptr<T>> {
    static constexpr auto value = true;
  };

  /** Returns the type resulting from a dereference operation. */
  template<typename T>
  struct DereferenceType {
    using type = std::decay_t<decltype(*std::declval<T>())>;
  };

  template<typename T>
  using GetDereferenceType = typename DereferenceType<T>::type;

  /**
   * Returns the type resulting from a dereference operation or the type itself
   * if it can't be dereferenced.
   */
  template<typename T>
  struct TryDereferenceType {
    private:
      template<typename U, bool Enabled>
      struct TryDereferenceHelper {
        using type = GetDereferenceType<U>;
      };

      template<typename U>
      struct TryDereferenceHelper<U, false> {
        using type = U;
      };

    public:
      using type = typename TryDereferenceHelper<std::decay_t<T>,
        IsDereferenceable<std::decay_t<T>>::value>::type;
  };

  template<typename T>
  using GetTryDereferenceType = typename TryDereferenceType<T>::type;

  template<typename T>
  decltype(auto) FullyDereference(T&& value) {
    if constexpr(IsDereferenceable<std::decay_t<T>>::value) {
      return FullyDereference(*value);
    } else {
      return value;
    }
  }
}

#endif
