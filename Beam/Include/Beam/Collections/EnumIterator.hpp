#ifndef BEAM_ENUM_ITERATOR_HPP
#define BEAM_ENUM_ITERATOR_HPP
#include <iterator>
#include "Beam/Collections/Enum.hpp"

namespace Beam {

  /**
   * Provides the ability to iterate over an enum type.
   * @tparam T The enum type to iterate over.
   */
  template<typename T>
  class EnumIterator {
    public:

      /** The enum type to iterate over. */
      using Type = T;

      using iterator_category = std::input_iterator_tag;
      using value_type = Type;
      using difference_type = Type;
      using pointer = Type*;
      using reference = Type&;

      /** Constructs an iterator to the first enumerated value. */
      EnumIterator() noexcept;

      /**
       * Constructs an iterator to a specified enumerated value.
       * @param current The current value to iterate over.
       */
      explicit EnumIterator(Type current) noexcept;

      Type operator *() const noexcept;
      EnumIterator operator ++(int) noexcept;
      EnumIterator operator ++() noexcept;
      bool operator ==(const EnumIterator& i) const = default;

    private:
      Type m_current;
  };

  /**
   * A sentinel class used to represent an enum range.
   * @tparam T The enum type to iterate over.
   */
  template<typename T>
  struct EnumRangeType {

    /** The enum type to iterate over. */
    using Type = T;
  };

  /** Constructs a range over an enum type. */
  template<typename T>
  EnumRangeType<T> make_range() {
    return EnumRangeType<T>();
  }

  /**
   * Returns an iterator to the beginning of an enum range.
   * @param value A dummy value used to represent the type of enum to iterate
   *        over.
   * @return An iterator to the beginning of the enum range.
   */
  template<typename T>
  EnumIterator<T> begin(EnumRangeType<T> value) noexcept {
    return EnumIterator(static_cast<typename EnumIterator<T>::Type>(0));
  }

  /**
   * Returns an iterator to the end of an enum range.
   * @param value A dummy value used to represent the type of enum to iterate
   *        over.
   * @return An iterator to the end of the enum range.
   */
  template<typename T>
  EnumIterator<T> end(EnumRangeType<T> value) noexcept {
    return EnumIterator(static_cast<typename EnumIterator<T>::Type>(
      EnumIterator<T>::Type::COUNT));
  }

  template<typename T>
  EnumIterator<T>::EnumIterator() noexcept
    : m_current(static_cast<Type>(0)) {}

  template<typename T>
  EnumIterator<T>::EnumIterator(Type current) noexcept
    : m_current(current) {}

  template<typename T>
  typename EnumIterator<T>::Type EnumIterator<T>::operator *() const noexcept {
    return m_current;
  }

  template<typename T>
  EnumIterator<T> EnumIterator<T>::operator ++(int) noexcept {
    auto i = *this;
    m_current = static_cast<Type>(static_cast<int>(m_current) + 1);
    return i;
  }

  template<typename T>
  EnumIterator<T> EnumIterator<T>::operator ++() noexcept {
    m_current = static_cast<Type>(static_cast<int>(m_current) + 1);
    return *this;
  }
}

#endif
