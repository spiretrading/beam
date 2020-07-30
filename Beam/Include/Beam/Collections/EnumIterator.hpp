#ifndef BEAM_ENUM_ITERATOR_HPP
#define BEAM_ENUM_ITERATOR_HPP
#include <iterator>
#include "Beam/Collections/Collections.hpp"
#include "Beam/Collections/Enum.hpp"

namespace Beam {

  /**
   * Provides the ability to iterate over an enum type.
   * @param <T> The enum type to iterate over.
   */
  template<typename T>
  class EnumIterator : public std::iterator<std::input_iterator_tag, T> {
    public:

      /** The enum type to iterate over. */
      using Type = T;

      /** Constructs an iterator to the first enumerated value. */
      EnumIterator();

      /**
       * Constructs an iterator to a specified enumerated value.
       * @param current The current value to iterate over.
       */
      explicit EnumIterator(Type current);

      /** Returns the current value. */
      Type operator *() const;

      /** Increments this iterator. */
      void operator ++(int);

      /** Increments this iterator. */
      EnumIterator operator ++();

      /**
       * Tests whether two iterators are pointing to the same value.
       * @param i The iterator to compare for equality.
       * @return <code>true</code> iff <code>this</code> iterator is pointing
       *         to the same value as <i>i</i>.
       */
      bool operator ==(const EnumIterator& i) const;

      /**
       * Tests whether two iterators are pointing to different values.
       * @param i The iterator to compare for inequality.
       * @return <code>true</code> iff <code>this</code> iterator is pointing
       *         to a different value than <i>i</i>.
       */
      bool operator !=(const EnumIterator& i) const;

    private:
      Type m_current;
  };

  /**
   * A sentinel class used to represent an enum range.
   * @param <T> The enum type to iterate over.
   */
  template<typename T>
  struct EnumRangeType {

    /** The enum type to iterate over. */
    using Type = T;
  };

  /** Constructs a range over an enum type. */
  template<typename T>
  EnumRangeType<T> MakeRange() {
    return EnumRangeType<T>();
  }

  /**
   * Returns an iterator to the beginning of an enum range.
   * @param value A dummy value used to represent the type of enum to iterate
   *        over.
   * @return An iterator to the beginning of the enum range.
   */
  template<typename T>
  EnumIterator<T> begin(EnumRangeType<T> value) {
    return EnumIterator(static_cast<typename EnumIterator<T>::Type>(0));
  }

  /**
   * Returns an iterator to the end of an enum range.
   * @param value A dummy value used to represent the type of enum to iterate
   *        over.
   * @return An iterator to the end of the enum range.
   */
  template<typename T>
  EnumIterator<T> end(EnumRangeType<T> value) {
    return EnumIterator(static_cast<typename EnumIterator<T>::Type>(
      EnumIterator<T>::Type::COUNT));
  }

  template<typename T>
  EnumIterator<T>::EnumIterator()
    : m_current(static_cast<Type>(0)) {}

  template<typename T>
  EnumIterator<T>::EnumIterator(Type current)
    : m_current(current) {}

  template<typename T>
  typename EnumIterator<T>::Type EnumIterator<T>::operator *() const {
    return m_current;
  }

  template<typename T>
  void EnumIterator<T>::operator ++(int) {
    m_current = static_cast<Type>(static_cast<int>(m_current) + 1);
  }

  template<typename T>
  EnumIterator<T> EnumIterator<T>::operator ++() {
    auto i = *this;
    m_current = static_cast<Type>(static_cast<int>(m_current) + 1);
    return i;
  }

  template<typename T>
  bool EnumIterator<T>::operator ==(const EnumIterator& i) const {
    return m_current == i.m_current;
  }

  template<typename T>
  bool EnumIterator<T>::operator !=(const EnumIterator& i) const {
    return !(*this == i);
  }
}

#endif
