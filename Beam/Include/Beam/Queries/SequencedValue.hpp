#ifndef BEAM_SEQUENCEDVALUE_HPP
#define BEAM_SEQUENCEDVALUE_HPP
#include <type_traits>
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Queries {

  /*! \struct SequencedValue
      \brief Stores a value that is part of a Sequence.
      \tparam T The value's data type.
   */
  template<typename T>
  class SequencedValue {
    public:

      //! The value's data type.
      using Value = T;

      //! Constructs a SequenceValue.
      SequencedValue();

      //! Converts from one type of SequencedValue to another.
      /*!
        \param value The value to convert.
      */
      template<typename U>
      SequencedValue(const SequencedValue<U>& value);

      //! Constructs a SequencedValue.
      /*!
        \param value The value to store.
        \param sequence The value's Sequence.
      */
      template<typename ValueForward>
      SequencedValue(ValueForward&& value, const Sequence& sequence);

      //! Returns the Value.
      const Value& GetValue() const;

      //! Returns the Value.
      Value& GetValue();

      //! Returns the Sequence.
      Sequence GetSequence() const;

      //! Returns the Sequence.
      Sequence& GetSequence();

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
      bool operator ==(const SequencedValue& rhs) const;

      //! Compares this value for inequality.
      /*!
        \param value The value to compare to for inequality.
        \return <code>true</code> iff the two values are not equal.
      */
      bool operator !=(const SequencedValue& rhs) const;

    private:
      friend struct Serialization::Shuttle<SequencedValue<T>>;
      Value m_value;
      Sequence m_sequence;
  };

  template<typename V>
  SequencedValue(V&& value, const Sequence& sequence) ->
    SequencedValue<std::decay_t<V>>;

  /*! \struct SequenceComparator
      \brief Defines a comparator for any two SequencedValues.
   */
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
  template<typename ValueForward>
  SequencedValue<T>::SequencedValue(ValueForward&& value,
      const Sequence& sequence)
    : m_value(std::forward<ValueForward>(value)),
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
  const typename SequencedValue<T>::Value& SequencedValue<T>::
      operator *() const {
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

  template<typename T>
  bool SequencedValue<T>::operator ==(const SequencedValue& rhs) const {
    return m_value == rhs.m_value && m_sequence == rhs.m_sequence;
  }

  template<typename T>
  bool SequencedValue<T>::operator !=(const SequencedValue& rhs) const {
    return !(*this == rhs);
  }

  template<typename T, typename Q>
  bool SequenceComparator::operator()(const SequencedValue<T>& lhs,
      const SequencedValue<Q>& rhs) const {
    return lhs.GetSequence() < rhs.GetSequence();
  }
}
}

namespace Beam {
namespace Serialization {
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
}

#endif
