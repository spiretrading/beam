#ifndef BEAM_LOCAL_PTR_HPP
#define BEAM_LOCAL_PTR_HPP
#include <type_traits>
#include <utility>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/Initializer.hpp"
#include "Beam/Pointers/NativePtr.hpp"
#include "Beam/Pointers/Pointers.hpp"
#include "Beam/Utilities/TypeList.hpp"

namespace Beam {

  /**
   * Wraps a local value as if it were a pointer.
   * @param <T> The type to wrap.
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
      LocalPtr(Args&&... args);

      /**
       * Constructs a LocalPtr.
       * @param initializer Stores the parameters used to initialize the value.
       */
      template<typename... Args>
      LocalPtr(Initializer<Args...>&& initializer);

      /** Tests if this LocalPtr is initialized. */
      explicit operator bool() const;

      /** Returns a reference to the value. */
      Type& operator *() const;

      /** Returns a pointer to the value. */
      Type* operator ->() const;

      /** Returns the value. */
      Type& Get() const;

    private:
      mutable Type m_value;

      template<typename Tuple, std::size_t... Sequence>
      LocalPtr(std::integer_sequence<std::size_t, Sequence...> sequence,
        Tuple&& args);
  };

  /**
   * Evaluates to a LocalPtr if T is not dereferenceable. Useful when a class
   * must always work using 'pointer' semantics but is using a value behind the
   * scenes.
   */
  template<typename T>
  struct OptionalLocalPtr {
    using type = std::conditional_t<IsDereferenceable<T>::value,
      GetOptionalNativePtr<T>, LocalPtr<T>>;
  };

  template<typename T>
  using GetOptionalLocalPtr = typename OptionalLocalPtr<T>::type;

  template<typename T>
  template<typename... Args, typename>
  LocalPtr<T>::LocalPtr(Args&&... args)
    : m_value(std::forward<Args>(args)...) {}

  template<typename T>
  template<typename... Args>
  LocalPtr<T>::LocalPtr(Initializer<Args...>&& args)
    : LocalPtr(std::make_integer_sequence<std::size_t, sizeof...(Args)>(),
        std::move(args.m_args)) {}

  template<typename T>
  LocalPtr<T>::operator bool() const {
    return true;
  }

  template<typename T>
  T& LocalPtr<T>::operator *() const {
    return m_value;
  }

  template<typename T>
  T* LocalPtr<T>::operator ->() const {
    return &m_value;
  }

  template<typename T>
  T& LocalPtr<T>::Get() const {
    return m_value;
  }

  template<typename T>
  template<typename Tuple, std::size_t... Sequence>
  LocalPtr<T>::LocalPtr(
    std::integer_sequence<std::size_t, Sequence...> sequence, Tuple&& args)
    : m_value(std::get<Sequence>(std::forward<Tuple>(args))...) {}
}

#endif
