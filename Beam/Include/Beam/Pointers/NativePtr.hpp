#ifndef BEAM_NATIVE_PTR_HPP
#define BEAM_NATIVE_PTR_HPP
#include "Beam/Pointers/Pointers.hpp"

namespace Beam {

  /** Wraps a native pointer. */
  template<typename T>
  class NativePtr {
    public:

      /**
       * Constructs a NativePtr storing a nullptr.
       */
      NativePtr();

      /**
       * Constructs a NativePtr.
       * @param ptr The pointer to hold.
       */
      NativePtr(T* ptr);

      /**
       * Copies a NativePtr.
       * @param ptr The pointer to copy.
       */
      NativePtr(const NativePtr& ptr) = default;

      /**
       * Moves a NativePtr.
       * @param ptr The NativePtr to move.
       */
      NativePtr(NativePtr&& ptr);

      /** Tests if this NativePtr is nullptr. */
      explicit operator bool() const;

      /** Returns a reference to the value. */
      T& operator *() const;

      /** Returns a pointer to the value. */
      T* operator ->() const;

      /**
       * Copies a NativePtr.
       * @param ptr The NativePtr to copy.
       */
      NativePtr& operator =(const NativePtr& ptr) = default;

      /**
       * Moves a NativePtr.
       * @param ptr The NativePtr to move.
       */
      NativePtr& operator =(NativePtr&& ptr);

    private:
      T* m_ptr;
  };

  /** Evaluates to a NativePtr if T is a raw pointer. */
  template<typename T>
  struct OptionalNativePtr {
    using type = T;
  };

  template<typename T>
  struct OptionalNativePtr<T*> {
    using type = NativePtr<T>;
  };

  template<typename T>
  using GetOptionalNativePtr = typename OptionalNativePtr<T>::type;

  template<typename T>
  NativePtr<T>::NativePtr()
    : m_ptr(nullptr) {}

  template<typename T>
  NativePtr<T>::NativePtr(T* ptr)
    : m_ptr(ptr) {}

  template<typename T>
  NativePtr<T>::NativePtr(NativePtr&& ptr)
      : m_ptr(ptr.m_ptr) {
    ptr.m_ptr = nullptr;
  }

  template<typename T>
  NativePtr<T>::operator bool() const {
    return static_cast<bool>(m_ptr);
  }

  template<typename T>
  T& NativePtr<T>::operator *() const {
    return *m_ptr;
  }

  template<typename T>
  T* NativePtr<T>::operator ->() const {
    return m_ptr;
  }

  template<typename T>
  NativePtr<T>& NativePtr<T>::operator =(NativePtr&& ptr) {
    m_ptr = ptr.m_ptr;
    ptr.m_ptr = nullptr;
    return *this;
  }
}

#endif
