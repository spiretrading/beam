#ifndef BEAM_SERIALIZED_VALUE_HPP
#define BEAM_SERIALIZED_VALUE_HPP
#include <cassert>
#include <concepts>
#include <memory>
#include <utility>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {

  /**
   * Delays the allocation/initialization of a serializable value.
   * @tparam T The type of the value.
   */
  template<typename T>
  class SerializedValue {
    public:

      /** The type of the value. */
      using Type = T;

      /** Constructs an uninitialized SerializedValue. */
      SerializedValue() noexcept;

      ~SerializedValue();

      /** Tests if this SerializedValue is initialized. */
      explicit operator bool() const;

      /** Returns a reference to the value. */
      Type& operator *() const;

      /** Returns a pointer to the value. */
      Type* operator ->() const;

      /** Returns <code>true</code> iff this is initialized. */
      bool is_initialized() const;

      /** Initializes the value. */
      void initialize();

      /** Returns the value. */
      Type& get() const;

      /** Resets the value. */
      void reset();

    private:
      friend struct DataShuttle;
      bool m_is_initialized;
      alignas(Type) unsigned char m_storage[sizeof(Type)];

      SerializedValue(const SerializedValue&) = delete;
      SerializedValue& operator =(const SerializedValue&) = delete;
  };

  template<typename T>
  SerializedValue<T>::SerializedValue() noexcept
    : m_is_initialized(false) {}

  template<typename T>
  SerializedValue<T>::~SerializedValue() {
    reset();
  }

  template<typename T>
  SerializedValue<T>::operator bool() const {
    return is_initialized();
  }

  template<typename T>
  typename SerializedValue<T>::Type& SerializedValue<T>::operator *() const {
    return get();
  }

  template<typename T>
  typename SerializedValue<T>::Type* SerializedValue<T>::operator ->() const {
    assert(m_is_initialized);
    return std::launder(reinterpret_cast<T*>(m_storage));
  }

  template<typename T>
  bool SerializedValue<T>::is_initialized() const {
    return m_is_initialized;
  }

  template<typename T>
  void SerializedValue<T>::initialize() {
    reset();
    BEAM_SUPPRESS_POD_INITIALIZER()
    new(m_storage) T(DataShuttle::make<T>());
    BEAM_UNSUPPRESS_POD_INITIALIZER()
    m_is_initialized = true;
  }

  template<typename T>
  typename SerializedValue<T>::Type& SerializedValue<T>::get() const {
    assert(m_is_initialized);
    return *std::launder(const_cast<T*>(reinterpret_cast<const T*>(m_storage)));
  }

  template<typename T>
  void SerializedValue<T>::reset() {
    if(!m_is_initialized) {
      return;
    }
    get().~T();
    m_is_initialized = false;
  }

  template<typename T> requires DataShuttle::is_default_constructible<T>
  class SerializedValue<T> {
    public:
      using Type = T;

      SerializedValue();

      explicit operator bool() const;
      Type& operator *() const;
      Type* operator ->() const;
      bool is_initialized() const;
      void initialize();
      Type& get() const;
      void reset();

    private:
      mutable Type m_value;

      SerializedValue(const SerializedValue&) = delete;
      SerializedValue& operator =(const SerializedValue&) = delete;
  };

  template<typename T> requires DataShuttle::is_default_constructible<T>
  SerializedValue<T>::SerializedValue()
    : m_value(DataShuttle::make<Type>()) {}

  template<typename T> requires DataShuttle::is_default_constructible<T>
  SerializedValue<T>::operator bool() const {
    return is_initialized();
  }

  template<typename T> requires DataShuttle::is_default_constructible<T>
  typename SerializedValue<T>::Type& SerializedValue<T>::operator *() const {
    return get();
  }

  template<typename T> requires DataShuttle::is_default_constructible<T>
  typename SerializedValue<T>::Type* SerializedValue<T>::operator ->() const {
    return &m_value;
  }

  template<typename T> requires DataShuttle::is_default_constructible<T>
  bool SerializedValue<T>::is_initialized() const {
    return true;
  }

  template<typename T> requires DataShuttle::is_default_constructible<T>
  void SerializedValue<T>::initialize() {}

  template<typename T> requires DataShuttle::is_default_constructible<T>
  typename SerializedValue<T>::Type& SerializedValue<T>::get() const {
    return m_value;
  }

  template<typename T> requires DataShuttle::is_default_constructible<T>
  void SerializedValue<T>::reset() {
    m_value.~T();
    std::construct_at(std::addressof(m_value));
  }
}

#endif
