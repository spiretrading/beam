#ifndef BEAM_INDEXEDVALUE_HPP
#define BEAM_INDEXEDVALUE_HPP
#include <ostream>
#include <utility>
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Queries {

  /*! \struct IndexedValue
      \brief Stores a value and its index.
      \tparam ValueType The value's data type.
      \tparam IndexType The index's data type.
   */
  template<typename ValueType, typename IndexType>
  class IndexedValue {
    public:

      //! The value's data type.
      using Value = ValueType;

      //! The index's data type.
      using Index = IndexType;

      //! Constructs an IndexedValue.
      IndexedValue();

      //! Converts from one type of IndexedValue to another.
      /*!
        \param value The value to convert.
      */
      template<typename U, typename K>
      IndexedValue(const IndexedValue<U, K>& value);

      //! Constructs an IndexedValue.
      /*!
        \param value The value to store.
        \param index The <i>value</i>'s index.
      */
      template<typename ValueForward, typename IndexForward>
      IndexedValue(ValueForward&& value, IndexForward&& index);

      //! Returns the Value.
      Value& GetValue();

      //! Returns the Value.
      const Value& GetValue() const;

      //! Returns the Index.
      Index& GetIndex();

      //! Returns the Index.
      const Index& GetIndex() const;

      //! Implicitly converts this to the value it represents.
      operator const Value& () const;

      //! Implicitly converts this to the value it represents.
      operator Value& ();

      //! Returns a reference to the Value.
      const Value& operator *() const;

      //! Returns a pointer to the Value.
      const Value* operator ->() const;

      //! Returns a reference to the Value.
      Value& operator *();

      //! Returns a pointer to the Value.
      Value* operator ->();

      //! Compares this value for equality.
      /*!
        \param value The value to compare to for equality.
        \return <code>true</code> iff the two values are equal.
      */
      bool operator ==(const IndexedValue& rhs) const;

      //! Compares this value for inequality.
      /*!
        \param value The value to compare to for inequality.
        \return <code>true</code> iff the two values are not equal.
      */
      bool operator !=(const IndexedValue& rhs) const;

    private:
      friend struct Serialization::Shuttle<IndexedValue<ValueType, IndexType>>;
      Value m_value;
      Index m_index;
  };

  template<typename V, typename I>
  IndexedValue(V&& value, I&& index) ->
    IndexedValue<std::decay_t<V>, std::decay_t<I>>;

  template<typename Value, typename Index>
  struct TimestampAccessor<IndexedValue<Value, Index>> {
    decltype(auto) operator ()(const IndexedValue<Value, Index>& value) const {
      return GetTimestamp(value.GetValue());
    }
  };

  template<typename Value, typename Index>
  inline std::ostream& operator <<(std::ostream& out,
      const IndexedValue<Value, Index>& value) {
    return out << "(" << value.GetIndex() << " " << value.GetValue() << ")";
  }

  template<typename ValueType, typename IndexType>
  IndexedValue<ValueType, IndexType>::IndexedValue()
      : m_value(),
        m_index() {}

  template<typename ValueType, typename IndexType>
  template<typename U, typename K>
  IndexedValue<ValueType, IndexType>::IndexedValue(
      const IndexedValue<U, K>& value)
      : m_value(value.GetValue()),
        m_index(value.GetIndex()) {}

  template<typename ValueType, typename IndexType>
  template<typename ValueForward, typename IndexForward>
  IndexedValue<ValueType, IndexType>::IndexedValue(ValueForward&& value,
      IndexForward&& index)
      : m_value(std::forward<ValueForward>(value)),
        m_index(std::forward<IndexForward>(index)) {}

  template<typename ValueType, typename IndexType>
  typename IndexedValue<ValueType, IndexType>::Value&
      IndexedValue<ValueType, IndexType>::GetValue() {
    return m_value;
  }

  template<typename ValueType, typename IndexType>
  const typename IndexedValue<ValueType, IndexType>::Value&
      IndexedValue<ValueType, IndexType>::GetValue() const {
    return m_value;
  }

  template<typename ValueType, typename IndexType>
  typename IndexedValue<ValueType, IndexType>::Index&
      IndexedValue<ValueType, IndexType>::GetIndex() {
    return m_index;
  }

  template<typename ValueType, typename IndexType>
  const typename IndexedValue<ValueType, IndexType>::Index&
      IndexedValue<ValueType, IndexType>::GetIndex() const {
    return m_index;
  }

  template<typename ValueType, typename IndexType>
  IndexedValue<ValueType, IndexType>::operator
      const typename IndexedValue<ValueType, IndexType>::Value& () const {
    return m_value;
  }

  template<typename ValueType, typename IndexType>
  IndexedValue<ValueType, IndexType>::operator
      typename IndexedValue<ValueType, IndexType>::Value& () {
    return m_value;
  }

  template<typename ValueType, typename IndexType>
  const typename IndexedValue<ValueType, IndexType>::Value&
      IndexedValue<ValueType, IndexType>::operator *() const {
    return m_value;
  }

  template<typename ValueType, typename IndexType>
  const typename IndexedValue<ValueType, IndexType>::Value*
      IndexedValue<ValueType, IndexType>::operator ->() const {
    return &m_value;
  }

  template<typename ValueType, typename IndexType>
  typename IndexedValue<ValueType, IndexType>::Value&
      IndexedValue<ValueType, IndexType>::operator *() {
    return m_value;
  }

  template<typename ValueType, typename IndexType>
  typename IndexedValue<ValueType, IndexType>::Value*
      IndexedValue<ValueType, IndexType>::operator ->() {
    return &m_value;
  }

  template<typename ValueType, typename IndexType>
  bool IndexedValue<ValueType, IndexType>::operator ==(
      const IndexedValue& rhs) const {
    return m_value == rhs.m_value && m_index == rhs.m_index;
  }

  template<typename ValueType, typename IndexType>
  bool IndexedValue<ValueType, IndexType>::operator !=(
      const IndexedValue& rhs) const {
    return !(*this == rhs);
  }
}
}

namespace Beam {
namespace Serialization {
  template<typename Value, typename Index>
  struct Shuttle<Beam::Queries::IndexedValue<Value, Index>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Beam::Queries::IndexedValue<Value, Index>& value,
        unsigned int version) {
      shuttle.Shuttle("value", value.m_value);
      shuttle.Shuttle("index", value.m_index);
    }
  };
}
}

#endif
