#ifndef BEAM_INDEXED_VALUE_HPP
#define BEAM_INDEXED_VALUE_HPP
#include <ostream>
#include <utility>
#include "Beam/Queries/Range.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /**
   * Stores a value and its index.
   * @tparam V The value's data type.
   * @tparam I The index's data type.
   */
  template<typename V, typename I>
  class IndexedValue {
    public:

      /** The value's data type. */
      using Value = V;

      /** The index's data type. */
      using Index = I;

      /** Constructs an IndexedValue. */
      IndexedValue() noexcept(std::is_nothrow_default_constructible_v<Value> &&
        std::is_nothrow_default_constructible_v<Index>);

      /**
       * Converts from one type of IndexedValue to another.
       * @param value The value to convert.
       */
      template<typename U, typename K>
      IndexedValue(const IndexedValue<U, K>& value) noexcept(
        std::is_nothrow_constructible_v<Value, const U&> &&
          std::is_nothrow_constructible_v<Index, const K&>);

      /**
       * Constructs an IndexedValue.
       * @param value The value to store.
       * @param index The <i>value</i>'s index.
       */
      template<typename VF, typename IF>
      IndexedValue(VF&& value, IF&& index) noexcept(
        std::is_nothrow_constructible_v<Value, VF&&> &&
          std::is_nothrow_constructible_v<Index, IF&&>);

      /** Returns the Value. */
      Value& get_value();

      /** Returns the Value. */
      const Value& get_value() const;

      /** Returns the Index. */
      Index& get_index();

      /** Returns the Index. */
      const Index& get_index() const;

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

      bool operator ==(const IndexedValue&) const = default;

    private:
      friend struct Shuttle<IndexedValue<V, I>>;
      Value m_value;
      Index m_index;
  };

  template<typename V, typename I>
  IndexedValue(V&& value, I&& index) ->
    IndexedValue<std::remove_cvref_t<V>, std::remove_cvref_t<I>>;

  template<typename Value, typename Index>
  struct TimestampAccessor<IndexedValue<Value, Index>> {
    decltype(auto) operator ()(const IndexedValue<Value, Index>& value) const {
      return get_timestamp(value.get_value());
    }
  };

  template<typename Value, typename Index>
  inline std::ostream& operator <<(
      std::ostream& out, const IndexedValue<Value, Index>& value) {
    return out << '(' << value.get_index() << ' ' << value.get_value() << ')';
  }

  template<typename V, typename I>
  IndexedValue<V, I>::IndexedValue() noexcept(
      std::is_nothrow_default_constructible_v<Value> &&
        std::is_nothrow_default_constructible_v<Index>)
    : m_value(),
      m_index() {}

  template<typename V, typename I>
  template<typename U, typename K>
  IndexedValue<V, I>::IndexedValue(const IndexedValue<U, K>& value) noexcept(
      std::is_nothrow_constructible_v<Value, const U&> &&
        std::is_nothrow_constructible_v<Index, const K&>)
    : m_value(value.get_value()),
      m_index(value.get_index()) {}

  template<typename V, typename I>
  template<typename VF, typename IF>
  IndexedValue<V, I>::IndexedValue(VF&& value, IF&& index) noexcept(
      std::is_nothrow_constructible_v<Value, VF&&> &&
        std::is_nothrow_constructible_v<Index, IF&&>)
    : m_value(std::forward<VF>(value)),
      m_index(std::forward<IF>(index)) {}

  template<typename V, typename I>
  typename IndexedValue<V, I>::Value& IndexedValue<V, I>::get_value() {
    return m_value;
  }

  template<typename V, typename I>
  const typename IndexedValue<V, I>::Value&
      IndexedValue<V, I>::get_value() const {
    return m_value;
  }

  template<typename V, typename I>
  typename IndexedValue<V, I>::Index& IndexedValue<V, I>::get_index() {
    return m_index;
  }

  template<typename V, typename I>
  const typename IndexedValue<V, I>::Index&
      IndexedValue<V, I>::get_index() const {
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

  template<typename Value, typename Index>
  struct Shuttle<Beam::IndexedValue<Value, Index>> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Beam::IndexedValue<Value, Index>& value,
        unsigned int version) const {
      shuttle.shuttle("value", value.m_value);
      shuttle.shuttle("index", value.m_index);
    }
  };
}

#endif
