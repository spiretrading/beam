#ifndef BEAM_QUERIES_VALUE_HPP
#define BEAM_QUERIES_VALUE_HPP
#include <memory>
#include <typeindex>
#include <boost/throw_exception.hpp>
#include "Beam/Serialization/ShuttleSharedPtr.hpp"
#include "Beam/Utilities/Streamable.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {
  class VirtualValue;

  /** Encapsulates a value used in a Query. */
  class Value : public Streamable {
    public:

      /**
       * Constructs a Value.
       * @param value Initializes the value.
       */
      template<DisableCopy<Value> T> requires(
        !std::convertible_to<T, std::string>)
      Value(T&& value);

      /**
       * Constructs a Value.
       * @param value Initializes the value.
       */
      Value(std::string value);

      Value(const Value&) = default;

      /** Returns the value's data type. */
      std::type_index get_type() const;

      /** Returns the value stored. */
      template<typename T>
      const T& as() const;

      template<typename T> requires(!std::convertible_to<T, std::string_view>)
      bool operator ==(const T& rhs) const;
      bool operator ==(std::string_view rhs) const;
      bool operator ==(const Value& rhs) const;

    private:
      friend struct DataShuttle;
      std::shared_ptr<VirtualValue> m_value;

      Value() = default;
      std::ostream& to_stream(std::ostream& out) const override;
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
      Value& operator =(const Value&) = delete;
  };

  /** Base class used to encapsulate a value used in a Query. */
  class VirtualValue : public Streamable {
    public:
      virtual ~VirtualValue() = default;

      /** Returns the value's data type. */
      virtual std::type_index get_type() const = 0;

      virtual bool operator ==(const VirtualValue& rhs) const = 0;

    protected:

      /** Constructs a VirtualValue. */
      VirtualValue() = default;

      VirtualValue(const VirtualValue&) = default;

    private:
      friend struct DataShuttle;

      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
      VirtualValue& operator =(const VirtualValue&) = delete;
  };

  /**
   * Stores a Value using a native type.
   * @tparam T The type of value to store.
   */
  template<typename T>
  class NativeValue : public VirtualValue {
    public:

      /** The type of value being stored. */
      using Type = T;

      /**
       * Constructs a NativeValue.
       * @param value Initializes the value.
       */
      template<DisableCopy<NativeValue<T>> V>
      NativeValue(V&& value);

      /** Returns the stored value. */
      const Type& get_value() const;

      std::type_index get_type() const override;
      bool operator ==(const VirtualValue& rhs) const override;

    private:
      friend struct DataShuttle;
      Type m_value;

      NativeValue() = default;
      std::ostream& to_stream(std::ostream& out) const override;
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<DisableCopy<Value> T> requires(!std::convertible_to<T, std::string>)
  Value::Value(T&& value)
    : m_value(std::make_shared<NativeValue<std::remove_cvref_t<T>>>(
        std::forward<T>(value))) {}

  inline Value::Value(std::string value)
    : m_value(std::make_shared<NativeValue<std::string>>(std::move(value))) {}

  inline std::type_index Value::get_type() const {
    return m_value->get_type();
  }

  template<typename T>
  const T& Value::as() const {
    if(m_value->get_type() != typeid(T)) {
      boost::throw_with_location(std::bad_cast());
    }
    return static_cast<const NativeValue<T>*>(m_value.get())->get_value();
  }

  template<typename T> requires(!std::convertible_to<T, std::string_view>)
  bool Value::operator ==(const T& rhs) const {
    if(m_value->get_type() != typeid(T)) {
      return false;
    }
    return static_cast<const NativeValue<T>*>(m_value.get())->get_value() ==
      rhs;
  }

  inline bool Value::operator ==(std::string_view rhs) const {
    if(m_value->get_type() != typeid(std::string)) {
      return false;
    }
    return static_cast<const NativeValue<std::string>*>(
      m_value.get())->get_value() == rhs;
  }

  inline bool Value::operator ==(const Value& rhs) const {
    return *m_value == *rhs.m_value;
  }

  inline std::ostream& Value::to_stream(std::ostream& out) const {
    return out << *m_value;
  }

  template<IsShuttle S>
  void Value::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("value", m_value);
  }

  template<IsShuttle S>
  void VirtualValue::shuttle(S& shuttle, unsigned int version) {}

  template<typename T>
  template<DisableCopy<NativeValue<T>> V>
  NativeValue<T>::NativeValue(V&& value)
    : m_value(std::forward<V>(value)) {}

  template<typename T>
  std::type_index NativeValue<T>::get_type() const {
    return typeid(Type);
  }

  template<typename T>
  const typename NativeValue<T>::Type& NativeValue<T>::get_value() const {
    return m_value;
  }

  template<typename T>
  bool NativeValue<T>::operator ==(const VirtualValue& rhs) const {
    if(get_type() != rhs.get_type()) {
      return false;
    }
    return m_value == static_cast<const NativeValue<T>*>(&rhs)->m_value;
  }

  template<typename T>
  std::ostream& NativeValue<T>::to_stream(std::ostream& out) const {
    if constexpr(std::is_same_v<T, std::string>) {
      return out << '\"' << m_value << '\"';
    } else if constexpr(std::is_same_v<T, bool>) {
      if(m_value) {
        return out << "true";
      }
      return out << "false";
    } else {
      return out << m_value;
    }
  }

  template<typename T>
  template<IsShuttle S>
  void NativeValue<T>::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("value", m_value);
  }
}

#endif
