#ifndef BEAM_INDEXED_VALUE_HPP
#define BEAM_INDEXED_VALUE_HPP
#include <ostream>
#include <utility>
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam::Queries {

  /**
   * Stores a value and its index.
   * @param <V> The value's data type.
   * @param <I> The index's data type.
   */
  template<typename V, typename I>
  class IndexedValue {
    public:

      /** The value's data type. */
      using Value = V;

      /** The index's data type. */
      using Index = I;

      /** Constructs an IndexedValue. */
      IndexedValue();

      /**
       * Converts from one type of IndexedValue to another.
       * @param value The value to convert.
       */
      template<typename U, typename K>
      IndexedValue(const IndexedValue<U, K>& value);

      /**
       * Constructs an IndexedValue.
       * @param value The value to store.
       * @param index The <i>value</i>'s index.
       */
      template<typename VF, typename IF>
      IndexedValue(VF&& value, IF&& index);

      /** Returns the Value. */
      Value& GetValue();

      /** Returns the Value. */
      const Value& GetValue() const;

      /** Returns the Index. */
      Index& GetIndex();

      /** Returns the Index. */
      const Index& GetIndex() const;

      /** Implicitly converts this to the value it represents. */
      operator const Value& () const;

      /** Implicitly converts this to the value it represents. */
      operator Value& ();

      /** Returns a reference to the Value. */
      const Value& operator *() const;

      /** Returns a pointer to the Value. */
      const Value* operator ->() const;

      /** Returns a reference to the Value. */
      Value& operator *();

      /** Returns a pointer to the Value. */
      Value* operator ->();

      /**
       * Compares this value for equality.
       * @param value The value to compare to for equality.
       * @return <code>true</code> iff the two values are equal.
       */
      bool operator ==(const IndexedValue& rhs) const;

      /**
       * Compares this value for inequality.
       * @param value The value to compare to for inequality.
       * @return <code>true</code> iff the two values are not equal.
       */
      bool operator !=(const IndexedValue& rhs) const;

    private:
      friend struct Serialization::Shuttle<IndexedValue<V, I>>;
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

  template<typename V, typename I>
  IndexedValue<V, I>::IndexedValue()
    : m_value(),
      m_index() {}

  template<typename V, typename I>
  template<typename U, typename K>
  IndexedValue<V, I>::IndexedValue(const IndexedValue<U, K>& value)
    : m_value(value.GetValue()),
      m_index(value.GetIndex()) {}

  template<typename V, typename I>
  template<typename VF, typename IF>
  IndexedValue<V, I>::IndexedValue(VF&& value, IF&& index)
    : m_value(std::forward<VF>(value)),
      m_index(std::forward<IF>(index)) {}

  template<typename V, typename I>
  typename IndexedValue<V, I>::Value& IndexedValue<V, I>::GetValue() {
    return m_value;
  }

  template<typename V, typename I>
  const typename IndexedValue<V, I>::Value&
      IndexedValue<V, I>::GetValue() const {
    return m_value;
  }

  template<typename V, typename I>
  typename IndexedValue<V, I>::Index& IndexedValue<V, I>::GetIndex() {
    return m_index;
  }

  template<typename V, typename I>
  const typename IndexedValue<V, I>::Index&
      IndexedValue<V, I>::GetIndex() const {
    return m_index;
  }

  template<typename V, typename I>
  IndexedValue<V, I>::operator
      const typename IndexedValue<V, I>::Value& () const {
    return m_value;
  }

  template<typename V, typename I>
  IndexedValue<V, I>::operator typename IndexedValue<V, I>::Value& () {
    return m_value;
  }

  template<typename V, typename I>
  const typename IndexedValue<V, I>::Value&
      IndexedValue<V, I>::operator *() const {
    return m_value;
  }

  template<typename V, typename I>
  const typename IndexedValue<V, I>::Value*
      IndexedValue<V, I>::operator ->() const {
    return &m_value;
  }

  template<typename V, typename I>
  typename IndexedValue<V, I>::Value& IndexedValue<V, I>::operator *() {
    return m_value;
  }

  template<typename V, typename I>
  typename IndexedValue<V, I>::Value* IndexedValue<V, I>::operator ->() {
    return &m_value;
  }

  template<typename V, typename I>
  bool IndexedValue<V, I>::operator ==(const IndexedValue& rhs) const {
    return m_value == rhs.m_value && m_index == rhs.m_index;
  }

  template<typename V, typename I>
  bool IndexedValue<V, I>::operator !=(const IndexedValue& rhs) const {
    return !(*this == rhs);
  }
}

namespace Beam::Serialization {
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

#endif
