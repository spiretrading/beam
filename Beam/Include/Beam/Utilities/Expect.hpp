#ifndef BEAM_EXPECT_HPP
#define BEAM_EXPECT_HPP
#include <concepts>
#include <exception>
#include <iostream>
#include <type_traits>
#include <utility>
#include <variant>

namespace Beam {

  /**
   * Stores a value that could potentially result in an exception.
   * @tparam T The type of value to store.
   */
  template<typename T>
  class Expect {
    public:

      /** The type of value to store. */
      using Type = T;

      /** Constructs an Expect. */
      Expect() = default;

      /**
       * Constructs an Expect with a normal value.
       * @param value The value to store.
       */
      Expect(const Type& value) noexcept(
        std::is_nothrow_copy_constructible_v<Type>);

      /**
       * Constructs an Expect with a normal value.
       * @param value The value to store.
       */
      Expect(Type&& value) noexcept(std::is_nothrow_move_constructible_v<Type>);

      /**
       * Constructs an Expect with an exception.
       * @param exception The exception to throw.
       */
      Expect(const std::exception_ptr& exception) noexcept;

      /** Implicitly converts to the underlying value. */
      operator const Type& () const;

      /** Returns <code>true</code> iff a value is stored. */
      bool is_value() const;

      /** Returns <code>true</code> iff an exception is stored. */
      bool is_exception() const;

      /** Returns the stored value, or throws an exception. */
      const Type& get() const&;

      /** Returns the stored value, or throws an exception. */
      Type& get() &;

      /** Returns the stored value, or throws an exception. */
      Type&& get() &&;

      /** Returns the exception. */
      std::exception_ptr get_exception() const;

      /**
       * Calls a function and stores its value.
       * @param f The function to call.
       */
      template<std::invocable<> F>
      void try_call(F&& f);

      template<typename U>
      Expect& operator =(const Expect<U>& rhs) noexcept(
        std::is_nothrow_assignable_v<Type, const U&>);

      template<typename U>
      Expect& operator =(Expect<U>&& rhs) noexcept(
        std::is_nothrow_assignable_v<Type, U&&>);

      template<typename U>
      Expect& operator =(const U& rhs) noexcept(
        std::is_nothrow_assignable_v<Type, const U&>);

      template<typename U>
      Expect& operator =(U&& rhs) noexcept(
        std::is_nothrow_assignable_v<Type, U&&>);

    private:
      template<typename> friend class Expect;
      std::variant<Type, std::exception_ptr> m_value;
  };

  /**
   * Stores a value that could potentially result in an exception.
   * @tparam T The type of value to store.
   */
  template<>
  class Expect<void> {
    public:

      /** The type of value to store. */
      using Type = void;

      /** Constructs an Expect. */
      Expect() = default;

      /**
       * Constructs an Expect with an exception.
       * @param exception The exception to throw.
       */
      Expect(const std::exception_ptr& exception) noexcept;

      /** Returns <code>true</code> iff a value is stored. */
      bool is_value() const;

      /** Returns <code>true</code> iff an exception is stored. */
      bool is_exception() const;

      /** Returns the stored value, or throws an exception. */
      void get() const;

      /** Returns the exception. */
      std::exception_ptr get_exception() const;

      /**
       * Calls a function and stores its value.
       * @param f The function to call.
       */
      template<std::invocable<> F>
      void try_call(F&& f);

    private:
      std::exception_ptr m_exception;
  };

  /**
   * Tries calling a function, capturing any thrown exception.
   * @param f The function to call.
   * @return The result of <i>f</i>.
   */
  template<std::invocable<> F>
  auto try_call(F&& f) noexcept {
    using Result = std::remove_cvref_t<std::invoke_result_t<F>>;
    try {
      if constexpr(std::same_as<Result, void>) {
        std::forward<F>(f)();
        return Expect<void>();
      } else {
        return Expect<Result>(std::forward<F>(f)());
      }
    } catch(...) {
      return Expect<Result>(std::current_exception());
    }
  }

  /**
   * Calls a function and if it throws an exception, nests the exception within
   * another.
   * @param f The function to call.
   * @param e The outer exception used if <i>f</i> throws.
   * @return The result of f.
   */
  template<std::invocable<> F, typename E> requires
    std::is_base_of_v<std::exception, std::remove_cvref_t<E>>
  decltype(auto) try_or_nest(F&& f, E&& e) {
    try {
      return std::forward<F>(f)();
    } catch(...) {
      std::throw_with_nested(std::forward<E>(e));
    }
  }

  /**
   * Returns an std::exception_ptr representing an std::nested_exception that
   * is derived from a specified exception passed as an argument representing
   * the outer exception and the currently thrown exception representing the
   * inner/nested exception.
   * @param e The outer exception.
   * @return The std::exception_ptr representing an std::nested_exception.
   */
  template<typename E> requires
    std::is_base_of_v<std::exception, std::remove_cvref_t<E>>
  std::exception_ptr nest_current_exception(E&& e) {
    try {
      std::throw_with_nested(std::forward<E>(e));
    } catch(...) {
      return std::current_exception();
    }
    return std::exception_ptr();
  }

  template<typename T>
  Expect<T>::Expect(const Type& value) noexcept(
    std::is_nothrow_copy_constructible_v<Type>)
    : m_value(value) {}

  template<typename T>
  Expect<T>::Expect(Type&& value) noexcept(
    std::is_nothrow_move_constructible_v<Type>)
    : m_value(std::move(value)) {}

  template<typename T>
  Expect<T>::Expect(const std::exception_ptr& exception) noexcept
    : m_value(exception) {}

  template<typename T>
  Expect<T>::operator const typename Expect<T>::Type& () const {
    return get();
  }

  template<typename T>
  bool Expect<T>::is_value() const {
    return m_value.index() == 0;
  }

  template<typename T>
  bool Expect<T>::is_exception() const {
    return m_value.index() == 1;
  }

  template<typename T>
  const typename Expect<T>::Type& Expect<T>::get() const& {
    if(is_value()) {
      return std::get<Type>(m_value);
    }
    std::rethrow_exception(std::get<std::exception_ptr>(m_value));
    throw std::exception();
  }

  template<typename T>
  typename Expect<T>::Type& Expect<T>::get() & {
    if(is_value()) {
      return std::get<Type>(m_value);
    }
    std::rethrow_exception(std::get<std::exception_ptr>(m_value));
    throw std::exception();
  }

  template<typename T>
  typename Expect<T>::Type&& Expect<T>::get() && {
    if(is_value()) {
      return std::move(std::get<Type>(m_value));
    }
    std::rethrow_exception(std::get<std::exception_ptr>(m_value));
    throw std::exception();
  }

  template<typename T>
  std::exception_ptr Expect<T>::get_exception() const {
    if(is_exception()) {
      return std::get<std::exception_ptr>(m_value);
    }
    return std::exception_ptr();
  }

  template<typename T>
  template<std::invocable<> F>
  void Expect<T>::try_call(F&& f) {
    try {
      m_value = f();
    } catch(...) {
      m_value = std::current_exception();
    }
  }

  template<typename T>
  template<typename U>
  Expect<typename Expect<T>::Type>&
    Expect<T>::operator =(const Expect<U>& rhs) noexcept(
      std::is_nothrow_assignable_v<Type, const U&>) {
    m_value = rhs.m_value;
    return *this;
  }

  template<typename T>
  template<typename U>
  Expect<typename Expect<T>::Type>&
    Expect<T>::operator =(Expect<U>&& rhs) noexcept(
      std::is_nothrow_assignable_v<Type, U&&>) {
    m_value = std::move(rhs.m_value);
    return *this;
  }

  template<typename T>
  template<typename U>
  Expect<typename Expect<T>::Type>&
    Expect<T>::operator =(const U& rhs) noexcept(
      std::is_nothrow_assignable_v<Type, const U&>) {
    m_value = rhs;
    return *this;
  }

  template<typename T>
  template<typename U>
  Expect<typename Expect<T>::Type>&
    Expect<T>::operator =(U&& rhs) noexcept(
      std::is_nothrow_assignable_v<Type, U&&>) {
    m_value = std::move(rhs);
    return *this;
  }

  inline Expect<void>::Expect(const std::exception_ptr& exception) noexcept
    : m_exception(exception) {}

  inline bool Expect<void>::is_value() const {
    return !m_exception;
  }

  inline bool Expect<void>::is_exception() const {
    return m_exception != nullptr;
  }

  inline void Expect<void>::get() const {
    if(m_exception) {
      std::rethrow_exception(m_exception);
    }
  }

  inline std::exception_ptr Expect<void>::get_exception() const {
    return m_exception;
  }

  template<std::invocable<> F>
  void Expect<void>::try_call(F&& f) {
    try {
      std::forward<F>(f)();
      m_exception = std::exception_ptr();
    } catch(...) {
      m_exception = std::current_exception();
    }
  }
}

#endif
