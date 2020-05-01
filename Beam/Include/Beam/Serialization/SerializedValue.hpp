#ifndef BEAM_SERIALIZEDVALUE_HPP
#define BEAM_SERIALIZEDVALUE_HPP
#include <cassert>
#include <new>
#include <utility>
#include "Beam/Serialization/Serialization.hpp"

namespace Beam {
namespace Serialization {

  /*! \class SerializedValue
      \brief Delays the allocation/initialization of a serializable value.
   */
  template<typename T>
  class SerializedValue {
    public:

      //! Constructs a SerializedValue.
      SerializedValue();

      ~SerializedValue();

      //! Tests if this SerializedValue is initialized.
      operator bool() const;

      //! Returns a reference to the value.
      T& operator *() const;

      //! Returns a pointer to the value.
      T* operator ->() const;

      //! Returns the value.
      T& Get() const;

      //! Initializes a the SerializedValue.
      void Initialize();

      //! Returns <code>true</code> iff this is initialized.
      bool IsInitialized() const;

      //! Resets the value.
      void Reset();

    private:
      friend struct DataShuttle;
      using Storage = typename std::aligned_storage<sizeof(T),
        std::alignment_of<T>::value>::type;
      Storage m_storage;
      T* m_ptr;
  };

  template<typename T>
  SerializedValue<T>::SerializedValue()
      : m_ptr(nullptr) {}

  template<typename T>
  SerializedValue<T>::~SerializedValue() {
    Reset();
  }

  template<typename T>
  SerializedValue<T>::operator bool() const {
    return m_ptr != nullptr;
  }

  template<typename T>
  T& SerializedValue<T>::operator *() const {
    assert(m_ptr != nullptr);
    return *m_ptr;
  }

  template<typename T>
  T* SerializedValue<T>::operator ->() const {
    assert(m_ptr != nullptr);
    return m_ptr;
  }

  template<typename T>
  T& SerializedValue<T>::Get() const {
    assert(m_ptr != nullptr);
    return *m_ptr;
  }

  template<typename T>
  bool SerializedValue<T>::IsInitialized() const {
    return m_ptr != nullptr;
  }

  template<typename T>
  void SerializedValue<T>::Reset() {
    if(m_ptr == nullptr) {
      return;
    }
    m_ptr->~T();
    m_ptr = nullptr;
  }
}
}

#endif
