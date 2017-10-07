#ifndef BEAM_QUERYNATIVEVALUE_HPP
#define BEAM_QUERYNATIVEVALUE_HPP
#include <type_traits>
#include <utility>
#include "Beam/Queries/NativeDataType.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Value.hpp"

namespace Beam {
namespace Queries {

  /*! \class NativeValue
      \brief Stores a Value using a native type.
      \tparam T The DataType represented.
   */
  template<typename T>
  class NativeValue : public VirtualValue,
      public CloneableMixin<NativeValue<T>> {
    public:

      //! The DataType represented.
      using Type = T;

      //! Constructs a NativeValue.
      NativeValue();

      //! Copies a NativeValue.
      /*!
        \param value The value to copy.
      */
      NativeValue(const NativeValue& value) = default;

      //! Constructs a NativeValue.
      /*!
        \param value Initializes the value.
      */
      template<typename ValueForward>
      explicit NativeValue(ValueForward&& value);

      virtual ~NativeValue() = default;

      virtual const DataType& GetType() const;

      //! Compares two NativeValues for equality.
      /*!
        \param value The value to compare to.
        \return <code>true</code> iff the value wrapped by <i>this</i> is equal
                to the value wrapped by <i>value</i>.
      */
      bool operator ==(const NativeValue& value) const;

      //! Compares two NativeValues for inequality.
      /*!
        \param value The value to compare to.
        \return <code>true</code> iff the value wrapped by <i>this</i> is
                not equal to the value wrapped by <i>value</i>.
      */
      bool operator !=(const NativeValue& value) const;

    protected:
      virtual const void* GetValuePtr() const;

      virtual std::ostream& ToStream(std::ostream& out) const;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      friend struct Beam::Serialization::DataShuttle;
      DataType m_type;
      typename Type::Type m_value;
  };

  //! Builds a NativeValue wrapping a value.
  /*!
    \param value The value to wrap.
  */
  template<typename T>
  NativeValue<NativeDataType<typename std::decay<T>::type>> MakeNativeValue(
      T&& value) {
    return NativeValue<NativeDataType<typename std::decay<T>::type>>(
      std::forward<T>(value));
  }

  template<typename T>
  NativeValue<T>::NativeValue()
      : m_value(),
        m_type{Type::GetInstance()} {}

  template<typename T>
  template<typename ValueForward>
  NativeValue<T>::NativeValue(ValueForward&& value)
      : m_value(std::forward<ValueForward>(value)),
        m_type{Type::GetInstance()} {}

  template<typename T>
  const DataType& NativeValue<T>::GetType() const {
    return m_type;
  }

  template<typename T>
  bool NativeValue<T>::operator ==(const NativeValue& value) const {
    return m_value == value.m_value;
  }

  template<typename T>
  bool NativeValue<T>::operator !=(const NativeValue& value) const {
    return !(*this == value);
  }

  template<typename T>
  const void* NativeValue<T>::GetValuePtr() const {
    return &m_value;
  }

  template<typename T>
  std::ostream& NativeValue<T>::ToStream(std::ostream& out) const {
    return out << m_value;
  }

  template<typename T>
  template<typename Shuttler>
  void NativeValue<T>::Shuttle(Shuttler& shuttle, unsigned int version) {
    VirtualValue::Shuttle(shuttle, version);
    shuttle.Shuttle("value", m_value);
  }
}
}

#endif
