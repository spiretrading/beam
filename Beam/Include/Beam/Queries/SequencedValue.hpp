#ifndef BEAM_SEQUENCED_VALUE_HPP
#define BEAM_SEQUENCED_VALUE_HPP
#include <type_traits>
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam::Queries {

  /**
   * Stores a value that is part of a Sequence.
   * @param <T> The value's data type.
   */
  template<typename T>
  class SequencedValue {
    public:

      /** The value's data type. */
      using Value = T;

      /** Constructs a SequenceValue. */
      SequencedValue();

      /**
       * Converts from one type of SequencedValue to another.
       * @param value The value to convert.
       */
      template<typename U>
      SequencedValue(const SequencedValue<U>& value);

      /**
       * Constructs a SequencedValue.
       * @param value The value to store.
       * @param sequence The value's Sequence.
       */
      template<typename VF>
      SequencedValue(VF&& value, Sequence sequence);

      /** Returns the Value. */
      const Value& GetValue() const;

      /** Returns the Value. */
      Value& GetValue();

      /** Returns the Sequence. */
      Sequence GetSequence() const;

      /** Returns the Sequence. */
      Sequence& GetSequence();

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

      bool operator ==(const SequencedValue& rhs) const = default;

    private:
      friend struct Serialization::Shuttle<SequencedValue<T>>;
      Value m_value;
      Sequence m_sequence;
  };

  template<typename V>
  SequencedValue(V&& value, const Sequence& sequence) ->
    SequencedValue<std::decay_t<V>>;

  /** Defines a comparator for any two SequencedValues. */
  struct SequenceComparator {
    template<typename T, typename Q>
    bool operator()(const SequencedValue<T>& lhs,
      const SequencedValue<Q>& rhs) const;
  };

  template<typename T>
  struct TimestampAccessor<SequencedValue<T>> {
    decltype(auto) operator ()(const SequencedValue<T>& value) const {
      return GetTimestamp(value.GetValue());
    }
  };

  template<typename T>
  inline std::ostream& operator <<(std::ostream& out,
      const SequencedValue<T>& value) {
    return out << "(" << value.GetValue() << " " << value.GetSequence() << ")";
  }

  template<typename T>
  SequencedValue<T>::SequencedValue()
    : m_value() {}

  template<typename T>
  template<typename U>
  SequencedValue<T>::SequencedValue(const SequencedValue<U>& value)
    : m_value(value.GetValue()),
      m_sequence(value.GetSequence()) {}

  template<typename T>
  template<typename VF>
  SequencedValue<T>::SequencedValue(VF&& value, Sequence sequence)
    : m_value(std::forward<VF>(value)),
      m_sequence(sequence) {}

  template<typename T>
  const typename SequencedValue<T>::Value& SequencedValue<T>::GetValue() const {
    return m_value;
  }

  template<typename T>
  typename SequencedValue<T>::Value& SequencedValue<T>::GetValue() {
    return m_value;
  }

  template<typename T>
  Sequence SequencedValue<T>::GetSequence() const {
    return m_sequence;
  }

  template<typename T>
  Sequence& SequencedValue<T>::GetSequence() {
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
  bool SequenceComparator::operator()(
      const SequencedValue<T>& lhs, const SequencedValue<Q>& rhs) const {
    return lhs.GetSequence() < rhs.GetSequence();
  }
}

namespace Beam::Serialization {
  template<typename T>
  struct Shuttle<Beam::Queries::SequencedValue<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Beam::Queries::SequencedValue<T>& value,
        unsigned int version) {
      shuttle.Shuttle("value", value.m_value);
      shuttle.Shuttle("sequence", value.m_sequence);
    }
  };
}

#endif
