#ifndef BEAM_SEQUENCED_VALUE_HPP
#define BEAM_SEQUENCED_VALUE_HPP
#include <type_traits>
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /**
   * Stores a value that is part of a Sequence.
   * @tparam T The value's data type.
   */
  template<typename T>
  class SequencedValue {
    public:

      /** The value's data type. */
      using Value = T;

      /** Constructs a SequenceValue. */
      SequencedValue() noexcept(std::is_nothrow_default_constructible_v<T>);

      /**
       * Converts from one type of SequencedValue to another.
       * @param value The value to convert.
       */
      template<typename U>
      SequencedValue(const SequencedValue<U>& value) noexcept(
        std::is_nothrow_constructible_v<T, const U&>);

      /**
       * Constructs a SequencedValue.
       * @param value The value to store.
       * @param sequence The value's Sequence.
       */
      template<typename VF>
      SequencedValue(VF&& value, Sequence sequence) noexcept(
        std::is_nothrow_constructible_v<T, VF&&>);

      /** Returns the Value. */
      const Value& get_value() const;

      /** Returns the Value. */
      Value& get_value();

      /** Returns the Sequence. */
      Sequence get_sequence() const;

      /** Returns the Sequence. */
      Sequence& get_sequence();

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

      bool operator ==(const SequencedValue&) const = default;

    private:
      friend struct Shuttle<SequencedValue<T>>;
      Value m_value;
      Sequence m_sequence;
  };

  template<typename V>
  SequencedValue(V&& value, const Sequence& sequence) ->
    SequencedValue<std::remove_cvref_t<V>>;

  /** Defines a comparator for any two SequencedValues. */
  struct SequenceComparator {
    template<typename T, typename Q>
    bool operator()(const SequencedValue<T>& lhs,
      const SequencedValue<Q>& rhs) const noexcept;
  };

  template<typename T>
  struct TimestampAccessor<SequencedValue<T>> {
    decltype(auto) operator ()(const SequencedValue<T>& value) const {
      return get_timestamp(value.get_value());
    }
  };

  template<typename T>
  inline std::ostream& operator <<(
      std::ostream& out, const SequencedValue<T>& value) {
    return out << '(' << value.get_value() << " " << value.get_sequence() <<
      ')';
  }

  template<typename T>
  SequencedValue<T>::SequencedValue() noexcept(
    std::is_nothrow_default_constructible_v<T>)
    : m_value() {}

  template<typename T>
  template<typename U>
  SequencedValue<T>::SequencedValue(const SequencedValue<U>& value) noexcept(
    std::is_nothrow_constructible_v<T, const U&>)
    : m_value(value.get_value()),
      m_sequence(value.get_sequence()) {}

  template<typename T>
  template<typename VF>
  SequencedValue<T>::SequencedValue(VF&& value, Sequence sequence) noexcept(
    std::is_nothrow_constructible_v<T, VF&&>)
    : m_value(std::forward<VF>(value)),
      m_sequence(sequence) {}

  template<typename T>
  const typename SequencedValue<T>::Value&
      SequencedValue<T>::get_value() const {
    return m_value;
  }

  template<typename T>
  typename SequencedValue<T>::Value& SequencedValue<T>::get_value() {
    return m_value;
  }

  template<typename T>
  Sequence SequencedValue<T>::get_sequence() const {
    return m_sequence;
  }

  template<typename T>
  Sequence& SequencedValue<T>::get_sequence() {
    return m_sequence;
  }

  template<typename T>
  SequencedValue<T>::operator
      const typename SequencedValue<T>::Value& () const {
    return m_value;
  }

  template<typename T>
  SequencedValue<T>::operator typename SequencedValue<T>::Value& () {
    return m_value;
  }

  template<typename T>
  const typename SequencedValue<T>::Value&
      SequencedValue<T>::operator *() const {
    return m_value;
  }

  template<typename T>
  const typename SequencedValue<T>::Value*
      SequencedValue<T>::operator ->() const {
    return &m_value;
  }

  template<typename T>
  typename SequencedValue<T>::Value& SequencedValue<T>::operator *() {
    return m_value;
  }

  template<typename T>
  typename SequencedValue<T>::Value* SequencedValue<T>::operator ->() {
    return &m_value;
  }

  template<typename T, typename Q>
  bool SequenceComparator::operator()(const SequencedValue<T>& lhs,
      const SequencedValue<Q>& rhs) const noexcept {
    return lhs.get_sequence() < rhs.get_sequence();
  }

  template<typename T>
  struct Shuttle<Beam::SequencedValue<T>> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Beam::SequencedValue<T>& value,
        unsigned int version) const {
      shuttle.shuttle("value", value.m_value);
      shuttle.shuttle("sequence", value.m_sequence);
    }
  };
}

#endif
