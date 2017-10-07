#ifndef BEAM_REF_HPP
#define BEAM_REF_HPP
#include <functional>
#include <boost/noncopyable.hpp>
#include "Beam/Pointers/Pointers.hpp"

namespace Beam {

  /*! \class RefType
      \brief Used to explicitly pass a reference.
   */
  template<typename T>
  class RefType {
    public:

      //! The type being referenced.
      using Type = T;

      //! Allows for polymorphic RefTypes.
      template<typename U>
      RefType(const RefType<U>& reference);

      //! Copies a RefType, workaround for non-compliant compilers;
      /*!
        \param ref The RefType to copy.
      */
      RefType(const RefType& ref);

      //! Acquires a RefType.
      /*!
        \param ref The RefType to acquire.
      */
      RefType(RefType&& ref);

      //! Acquires a RefType.
      /*!
        \param ref The RefType to acquire.
      */
      RefType& operator =(RefType&& ref);

      //! Returns a reference to the result.
      Type& operator *();

      //! Returns a reference to the result.
      const Type& operator *() const;

      //! Returns a pointer to the result.
      Type* operator ->();

      //! Returns a pointer to the result.
      const Type* operator ->() const;

      //! Returns a pointer to the result.
      Type* Get();

      //! Returns a pointer to the result.
      const Type* Get() const;

    private:
      template<typename> friend class RefType;
      template<typename U> friend RefType<U> Ref(U&);
      template<typename U> friend RefType<U> Ref(RefType<U>&);
      Type* m_reference;

      RefType(Type& reference);
  };

  //! Returns a RefType for a given value.
  /*!
    \param reference A reference to the value to wrap.
    \return A RefType carrying the <i>reference</i>.
  */
  template<typename T>
  RefType<T> Ref(T& reference) {
    return RefType<T>(reference);
  }

  //! Returns a RefType for a given value.
  /*!
    \param reference A reference to the value to wrap.
    \return A RefType carrying the <i>reference</i>.
  */
  template<typename T>
  RefType<T> Ref(RefType<T>& reference) {
    return RefType<T>(*reference.Get());
  }

  template<typename T>
  RefType<T>::RefType(const RefType& ref)
      : m_reference{ref.m_reference} {}

  template<typename T>
  RefType<T>::RefType(RefType&& ref)
      : m_reference(ref.m_reference) {
    ref.m_reference = nullptr;
  }

  template<typename T>
  RefType<T>& RefType<T>::operator =(RefType&& ref) {
    if(this == &ref) {
      return *this;
    }
    m_reference = ref.m_reference;
    ref.m_reference = nullptr;
    return *this;
  }

  template<typename T>
  T& RefType<T>::operator *() {
    return *m_reference;
  }

  template<typename T>
  const T& RefType<T>::operator *() const {
    return *m_reference;
  }

  template<typename T>
  T* RefType<T>::operator ->() {
    return m_reference;
  }

  template<typename T>
  const T* RefType<T>::operator ->() const {
    return m_reference;
  }

  template<typename T>
  T* RefType<T>::Get() {
    return m_reference;
  }

  template<typename T>
  const T* RefType<T>::Get() const {
    return m_reference;
  }

  template<typename T>
  RefType<T>::RefType(T& reference)
      : m_reference(&reference) {}

  template<typename T>
  template<typename U>
  RefType<T>::RefType(const RefType<U>& reference)
      : m_reference(reference.m_reference) {}
}

#endif
