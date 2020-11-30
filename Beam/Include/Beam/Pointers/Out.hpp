#ifndef BEAM_OUT_HPP
#define BEAM_OUT_HPP
#include "Beam/Pointers/Pointers.hpp"

namespace Beam {

  /**
   * Used to identify a parameter as an output.
   * @param <T> The type of the parameter.
   */
  template<typename T>
  class Out {
    public:

      /** The type of the parameter. */
      using Type = T;

      /** Allows for polymorphic Out params. */
      template<typename U>
      Out(const Out<U>& out);

      /**
       * Copies an Out.
       * @param out The Out to copy.
       */
      Out(const Out& out);

      /**
       * Acquires an Out.
       * @param out The Out to acquire.
       */
      Out(Out&& out);

      /**
       * Acquires an Out.
       * @param out The Out to acquire.
       */
      Out& operator =(Out&& out);

      /** Returns <code>true</code> iff the result is ignored. */
      bool IsIgnored() const;

      /** Returns a reference to the result. */
      Type& operator *();

      /** Returns a reference to the result. */
      const Type& operator *() const;

      /** Returns a pointer to the result. */
      Type* operator ->();

      /** Returns a pointer to the result. */
      const Type* operator ->() const;

      /** Returns a pointer to the result. */
      Type* Get();

      /** Returns a pointer to the result. */
      const Type* Get() const;

    private:
      template<typename> friend class Out;
      template<typename U> friend Out<U> Store(U& result);
      template<typename U> friend Out<U> Store(Out<U>& result);
      Type* m_result;

      Out(Type* result);
  };

  /**
   * Indicates that a parameter is being used as the result of a function.
   * @param result Where the result of the function is stored.
   * @return An Out to the <i>result</i>.
   */
  template<typename T>
  Out<T> Store(T& result) {
    return Out<T>(&result);
  }

  /**
   * Indicates that a parameter is being used as the result of a function.
   * @param result Where the result of the function is stored.
   * @return An Out to the <i>result</i>.
   */
  template<typename T>
  Out<T> Store(Out<T>& result) {
    return Out<T>(result.Get());
  }

  template<typename T>
  template<typename U>
  Out<T>::Out(const Out<U>& out)
    : m_result(out.m_result) {}

  template<typename T>
  Out<T>::Out(const Out& out)
    : m_result(out.m_result) {}

  template<typename T>
  Out<T>::Out(Out&& out)
      : m_result(out.m_result) {
    out.m_result = nullptr;
  }

  template<typename T>
  Out<T>& Out<T>::operator =(Out&& out) {
    if(this == &out) {
      return *this;
    }
    m_result = out.m_result;
    out.m_result = nullptr;
    return *this;
  }

  template<typename T>
  bool Out<T>::IsIgnored() const {
    return m_result == nullptr;
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
  typename Out<T>::Type* Out<T>::Get() {
    return m_result;
  }

  template<typename T>
  const typename Out<T>::Type* Out<T>::Get() const {
    return m_result;
  }

  template<typename T>
  Out<T>::Out(T* result)
    : m_result(result) {}
}

#endif
