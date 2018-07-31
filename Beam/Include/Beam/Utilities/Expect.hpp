#ifndef BEAM_EXPECT_HPP
#define BEAM_EXPECT_HPP
#include <exception>
#include <utility>
#include <boost/variant/get.hpp>
#include <boost/variant/variant.hpp>
#include "Beam/Utilities/Functional.hpp"
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class Expect
      \brief Stores a value that could potentially result in an exception.
      \tparam T The type of value to store.
   */
  template<typename T>
  class Expect {
    public:

      //! The type of value to store.
      using Type = T;

      //! Constructs an Expect.
      Expect() = default;

      //! Constructs an Expect with a normal value.
      /*!
        \param value The value to store.
      */
      Expect(const T& value);

      //! Constructs an Expect with a normal value.
      /*!
        \param value The value to store.
      */
      Expect(T&& value);

      //! Constructs an Expect with an exception.
      /*!
        \param exception The exception to throw.
      */
      Expect(const std::exception_ptr& exception);

      //! Implicitly converts to the underlying value.
      operator const T& () const;

      //! Returns <code>true</code> iff a value is stored.
      bool IsValue() const;

      //! Returns <code>true</code> iff an exception is stored.
      bool IsException() const;

      //! Returns the stored value, or throws an exception.
      const T& Get() const;

      //! Returns the stored value, or throws an exception.
      T& Get();

      //! Returns the exception.
      std::exception_ptr GetException() const;

      //! Calls a function and stores its value.
      /*!
        \param f The function to call.
      */
      template<typename F>
      void Try(F&& f);

      template<typename U>
      Expect& operator =(const Expect<U>& rhs);

      template<typename U>
      Expect& operator =(Expect<U>&& rhs);

      template<typename U>
      Expect& operator =(const U& rhs);

      template<typename U>
      Expect& operator =(U&& rhs);

    private:
      template<typename> friend class Expect;
      boost::variant<T, std::exception_ptr> m_value;
  };

  /*! \class Expect
      \brief Stores a value that could potentially result in an exception.
      \tparam T The type of value to store.
   */
  template<>
  class Expect<void> {
    public:

      //! The type of value to store.
      using Type = void;

      //! Constructs an Expect.
      Expect() = default;

      //! Constructs an Expect with an exception.
      /*!
        \param exception The exception to throw.
      */
      Expect(const std::exception_ptr& exception);

      //! Returns <code>true</code> iff an exception is stored.
      bool IsException() const;

      //! Returns the stored value, or throws an exception.
      void Get() const;

      //! Returns the exception.
      std::exception_ptr GetException() const;

      //! Calls a function and stores its value.
      /*!
        \param f The function to call.
      */
      template<typename F>
      void Try(F&& f);

    private:
      std::exception_ptr m_exception;
  };

  //! Tries calling a function, capturing any thrown exception.
  /*!
    \param f The function to call.
    \return The result of <i>f</i>.
  */
  template<typename F>
  Expect<typename std::decay<GetResultOf<typename std::decay<F>::type>>::type>
      Try(F&& f) {
    using Result = typename std::decay<
      GetResultOf<typename std::decay<F>::type>>::type;
    try {
      if constexpr(std::is_same_v<Result, void>) {
        f();
        return {};
      } else {
        return f();
      }
    } catch(...) {
      return std::current_exception();
    }
  }

  //! Calls a function and calls terminate if the function throws an exception.
  /*!
    \param f The function to call.
    \param args The arguments to pass to f.
    \return The result of f.
  */
  template<typename F, typename... Args>
  auto Require(F&& f, Args&&... args) noexcept {
    try {
      return f(std::forward<Args>(args)...);
    } catch(const std::exception& e) {
      std::cerr << e.what();
    } catch(...) {
      std::cerr << "Unknown error.";
    }
    std::exit(-1);
    return f(std::forward<Args>(args)...);
  }

  template<typename T>
  Expect<T>::Expect(const T& value)
      : m_value(value) {}

  template<typename T>
  Expect<T>::Expect(T&& value)
      : m_value(std::move(value)) {}

  template<typename T>
  Expect<T>::Expect(const std::exception_ptr& exception)
      : m_value(exception) {}

  template<typename T>
  Expect<T>::operator const T& () const {
    return Get();
  }

  template<typename T>
  bool Expect<T>::IsValue() const {
    return m_value.which() == 0;
  }

  template<typename T>
  bool Expect<T>::IsException() const {
    return m_value.which() == 1;
  }

  template<typename T>
  const typename Expect<T>::Type& Expect<T>::Get() const {
    if(IsValue()) {
      return boost::get<Type>(m_value);
    }
    std::rethrow_exception(boost::get<std::exception_ptr>(m_value));
    throw std::exception();
  }

  template<typename T>
  typename Expect<T>::Type& Expect<T>::Get() {
    if(IsValue()) {
      return boost::get<Type>(m_value);
    }
    std::rethrow_exception(boost::get<std::exception_ptr>(m_value));
    throw std::exception();
  }

  template<typename T>
  std::exception_ptr Expect<T>::GetException() const {
    if(IsException()) {
      return boost::get<std::exception_ptr>(m_value);
    }
    return std::exception_ptr();
  }

  template<typename T>
  template<typename F>
  void Expect<T>::Try(F&& f) {
    try {
      m_value = f();
    } catch(...) {
      m_value = std::current_exception();
    }
  }

  template<typename T>
  template<typename U>
  Expect<T>& Expect<T>::operator =(const Expect<U>& rhs) {
    m_value = rhs.m_value;
    return *this;
  }

  template<typename T>
  template<typename U>
  Expect<T>& Expect<T>::operator =(Expect<U>&& rhs) {
    m_value = std::move(rhs.m_value);
    return *this;
  }

  template<typename T>
  template<typename U>
  Expect<T>& Expect<T>::operator =(const U& rhs) {
    m_value = rhs;
    return *this;
  }

  template<typename T>
  template<typename U>
  Expect<T>& Expect<T>::operator =(U&& rhs) {
    m_value = std::move(rhs);
    return *this;
  }

  inline Expect<void>::Expect(const std::exception_ptr& exception)
      : m_exception(exception) {}

  inline bool Expect<void>::IsException() const {
    return m_exception != std::exception_ptr();
  }

  inline void Expect<void>::Get() const {
    if(m_exception != std::exception_ptr()) {
      std::rethrow_exception(m_exception);
    }
  }

  inline std::exception_ptr Expect<void>::GetException() const {
    return m_exception;
  }

  template<typename F>
  void Expect<void>::Try(F&& f) {
    try {
      f();
      m_exception = std::exception_ptr();
    } catch(...) {
      m_exception = std::current_exception();
    }
  }
}

#endif
