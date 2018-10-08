#ifndef BEAM_REF_HPP
#define BEAM_REF_HPP
#include "Beam/Pointers/Pointers.hpp"

namespace Beam {

  /** Used to explicitly pass a reference. */
  template<typename T>
  class Ref {
    public:

      //! The type being referenced.
      using Type = T;

      //! Constructs a Ref.
      /*!
        \param reference The reference to wrap.
      */
      explicit Ref(Type& reference);

      //! Allows for polymorphic Refs.
      template<typename U>
      Ref(const Ref<U>& reference);

      //! Copies a Ref.
      Ref(const Ref& ref) = default;

      //! Acquires a Ref.
      /*!
        \param ref The Ref to acquire.
      */
      Ref(Ref&& ref);

      //! Acquires a Ref.
      /*!
        \param ref The Ref to acquire.
      */
      Ref& operator =(Ref&& ref);

      //! Returns a reference to the result.
      Type& operator *() const;

      //! Returns a pointer to the result.
      Type* operator ->() const;

      //! Returns a pointer to the result.
      Type* Get() const;

    private:
      Type* m_reference;
  };

  template<typename T>
  Ref<T>::Ref(Type& reference)
      : m_reference(&reference) {}

  template<typename T>
  template<typename U>
  Ref<T>::Ref(const Ref<U>& reference)
      : m_reference(reference.Get()) {}

  template<typename T>
  Ref<T>::Ref(Ref&& ref)
      : m_reference(ref.m_reference) {
    ref.m_reference = nullptr;
  }

  template<typename T>
  Ref<T>& Ref<T>::operator =(Ref&& ref) {
    if(this == &ref) {
      return *this;
    }
    m_reference = ref.m_reference;
    ref.m_reference = nullptr;
    return *this;
  }

  template<typename T>
  T& Ref<T>::operator *() const {
    return *m_reference;
  }

  template<typename T>
  T* Ref<T>::operator ->() const {
    return m_reference;
  }

  template<typename T>
  T* Ref<T>::Get() const {
    return m_reference;
  }
}

#endif
