#ifndef BEAM_OUT_HPP
#define BEAM_OUT_HPP
#include <concepts>
#include <utility>

namespace Beam {

  /**
   * Used to mark a parameter as an output.
   * @tparam T The type of the parameter.
   */
  template<typename T>
  class Out {
    public:

      /** The type of the parameter. */
      using Type = T;

      /**
       * Constructs an Out from an Out of a convertible type.
       * @param other The Out to convert from.
       */
      template<typename U> requires std::convertible_to<U*, T*>
      Out(const Out<U>& other) noexcept;

      /** Returns a pointer to the result. */
      Type* get();

      /** Returns a pointer to the result. */
      const Type* get() const;

      /** Returns a reference to the result. */
      Type& operator *();

      /** Returns a reference to the result. */
      const Type& operator *() const;

      /** Returns a pointer to the result. */
      Type* operator ->();

      /** Returns a pointer to the result. */
      const Type* operator ->() const;

    private:
      template<typename U> friend Out<U> out(U& result);
      template<typename U> friend Out<U> out(Out<U>& result);
      template<typename U> friend class Out;
      Type* m_result;

      explicit Out(Type& result) noexcept;
  };

  /**
   * Indicates that a parameter is being used as the result of a function.
   * @param result Where the result of the function is stored.
   * @return An Out to the <i>result</i>.
   */
  template<typename T>
  Out<T> out(T& result) {
    return Out(result);
  }

  /**
   * Indicates that a parameter is being used as the result of a function.
   * @param result Where the result of the function is stored.
   * @return An Out to the <i>result</i>.
   */
  template<typename T>
  Out<T> out(Out<T>& result) {
    return Out(*result.get());
  }

  template<typename T>
  template<typename U> requires std::convertible_to<U*, T*>
  Out<T>::Out(const Out<U>& other) noexcept
    : m_result(other.m_result) {}

  template<typename T>
  typename Out<T>::Type* Out<T>::get() {
    return m_result;
  }

  template<typename T>
  const typename Out<T>::Type* Out<T>::get() const {
    return m_result;
  }

  template<typename T>
  typename Out<T>::Type& Out<T>::operator *() {
    return *m_result;
  }

  template<typename T>
  const typename Out<T>::Type& Out<T>::operator *() const {
    return *m_result;
  }

  template<typename T>
  typename Out<T>::Type* Out<T>::operator ->() {
    return m_result;
  }

  template<typename T>
  const typename Out<T>::Type* Out<T>::operator ->() const {
    return m_result;
  }

  template<typename T>
  Out<T>::Out(Type& result) noexcept
    : m_result(&result) {}
}

#endif
