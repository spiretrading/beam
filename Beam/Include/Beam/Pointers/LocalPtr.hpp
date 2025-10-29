#ifndef BEAM_LOCAL_PTR_HPP
#define BEAM_LOCAL_PTR_HPP
#include <type_traits>
#include <utility>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/Initializer.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Wraps a local value as if it were a pointer.
   * @tparam T The type to wrap.
   */
  template<typename T>
  class LocalPtr {
    public:

      /** The type being wrapped. */
      using Type = T;

      /**
       * Constructs a LocalPtr.
       * @param args The parameters used to initialize the value.
       */
      template<typename... Args, typename =
        disable_copy_constructor_t<LocalPtr, Args...>>
      LocalPtr(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<Type, Args&&...>);

      /**
       * Constructs a LocalPtr.
       * @param initializer Stores the parameters used to initialize the value.
       */
      template<typename... Args>
      LocalPtr(Initializer<Args...>&& initializer) noexcept(
        std::is_nothrow_constructible_v<Type, Args...>);

      /** Tests if this LocalPtr is initialized. */
      explicit operator bool() const;

      /** Returns the value. */
      Type* get() const;

      /** Returns a reference to the value. */
      Type& operator *() const;

      /** Returns a pointer to the value. */
      Type* operator ->() const;

    private:
      mutable Type m_value;
  };

  /**
   * Evaluates to a LocalPtr if T is not dereferenceable. Useful when a class
   * must always work using 'pointer' semantics but is using a value behind the
   * scenes.
   */
  template<typename T>
  struct local_ptr {
    using type = std::conditional_t<IsDereferenceable<T>, T, LocalPtr<T>>;
  };

  template<typename T>
  using local_ptr_t = typename local_ptr<T>::type;

  /**
   * Checks if a type can be used to construct another type or initialize a
   * pointer or indirect value of some type.
   * @tparam T The type to test.
   * @tparam U The type to initialize from T.
   */
  template<typename T, typename U>
  concept Initializes = std::constructible_from<local_ptr_t<U>, T&&>;

  /**
   * Helper function for capturing references inside lambda expressions.
   * Captures an lvalue reference as pointer and captures an rvalue reference by
   * taking ownership and wrapping it in a local pointer. In both cases pointer
   * semantics are used for the captured value.
   * @param value The value to capture.
   * @return A raw pointer if <i>value</i> is an lvalue reference, and a
   *         LocalPtr if <i>value</i> is an rvalue reference.
   */
  template<typename T>
  auto capture_ptr(std::remove_cvref_t<T>& value) {
    if constexpr(std::is_lvalue_reference_v<T>) {
      return &value;
    } else {
      return LocalPtr<std::remove_cvref_t<T>>(static_cast<T&&>(value));
    }
  }

  template<typename T>
  auto capture_ptr(std::remove_cvref_t<T>&& value) {
    static_assert(!std::is_lvalue_reference_v<T>,
      "Can not forward an rvalue as an lvalue.");
    return LocalPtr<std::remove_cvref_t<T>>(static_cast<T&&>(value));
  }

  template<typename T>
  template<typename... Args, typename>
  LocalPtr<T>::LocalPtr(Args&&... args) noexcept(
    std::is_nothrow_constructible_v<Type, Args&&...>)
    : m_value(std::forward<Args>(args)...) {}

  template<typename T>
  template<typename... Args>
  LocalPtr<T>::LocalPtr(Initializer<Args...>&& args) noexcept(
    std::is_nothrow_constructible_v<Type, Args...>)
    : m_value(make<Type>(std::move(args))) {}

  template<typename T>
  LocalPtr<T>::operator bool() const {
    return true;
  }

  template<typename T>
  T* LocalPtr<T>::get() const {
    return &m_value;
  }

  template<typename T>
  T& LocalPtr<T>::operator *() const {
    return m_value;
  }

  template<typename T>
  T* LocalPtr<T>::operator ->() const {
    return &m_value;
  }
}

#endif
