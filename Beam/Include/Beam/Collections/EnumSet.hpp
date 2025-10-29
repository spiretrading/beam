#ifndef BEAM_ENUM_SET_HPP
#define BEAM_ENUM_SET_HPP
#include <bitset>
#include <type_traits>
#include "Beam/Collections/Enum.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/ShuttleBitset.hpp"

namespace Beam {

  /**
   * Stores a set of enum values.
   * @tparam T The enum type to store.
   */
  template<typename T>
  class EnumSet {
    public:

      /** The enum type stored. */
      using Type = T;

      /**
       * Returns an EnumSet from its underlying representation.
       * @param value The value to build the EnumSet from.
       */
      static EnumSet from_bitmask(std::uint32_t value);

      /** Constructs an empty EnumSet. */
      EnumSet() = default;

      /**
       * Constructs an EnumSet containing a single value.
       * @param value The value to store.
       */
      EnumSet(Type value) noexcept;

      /**
       * Constructs an EnumSet containing a single value.
       * @param value The value to store.
       */
      EnumSet(typename Type::Type value) noexcept;

      /**
       * Constructs an EnumSet from a bitset.
       * @param set The bitset to represent.
       */
      explicit EnumSet(const std::bitset<Type::COUNT>& set) noexcept;

      /** Converts this set to a bitset. */
      explicit operator const std::bitset<T::COUNT>& () const;

      /** Converts this set to a bitset. */
      explicit operator std::bitset<T::COUNT>& ();

      /**
       * Tests if a value belongs to this set.
       * @param value The value to test.
       * @return <code>true</code> iff the <i>value</i> is a member of this set.
       */
      bool test(Type value) const;

      /**
       * Adds all values in a set to this set.
       * @param values The set of values to add to this set.
       */
      EnumSet& set(const EnumSet& values);

      /**
       * Removes a value from this set.
       * @param values The set of values to remove from this set.
       */
      EnumSet& reset(const EnumSet& values);

      /** Returns the bitset. */
      const std::bitset<T::COUNT>& get_bitset() const;

      bool operator ==(const EnumSet&) const = default;

    private:
      std::bitset<Type::COUNT> m_bitset;
  };

  template<typename T>
  std::ostream& operator <<(std::ostream& out, const EnumSet<T>& set) {
    return out << set.get_bitset();
  }

  template<typename T>
  EnumSet<T> operator &(const EnumSet<T>& lhs, const EnumSet<T>& rhs) {
    return EnumSet<T>(lhs.get_bitset() & rhs.get_bitset());
  }

  template<typename T>
  EnumSet<T> operator |(const EnumSet<T>& lhs, const EnumSet<T>& rhs) {
    return EnumSet<T>(lhs.get_bitset() | rhs.get_bitset());
  }

  template<typename T>
  EnumSet<T> operator ^(const EnumSet<T>& lhs, const EnumSet<T>& rhs) {
    return EnumSet<T>(lhs.get_bitset() ^ rhs.get_bitset());
  }

  template<typename T>
  EnumSet<T> EnumSet<T>::from_bitmask(std::uint32_t value) {
    auto set = EnumSet();
    set.m_bitset = value;
    return set;
  }

  template<typename T>
  EnumSet<T>::EnumSet(Type value) noexcept {
    if(value != Type::NONE) {
      m_bitset.set(static_cast<int>(value));
    }
  }

  template<typename T>
  EnumSet<T>::EnumSet(typename Type::Type value) noexcept
    : EnumSet(static_cast<Type>(value)) {}

  template<typename T>
  EnumSet<T>::EnumSet(const std::bitset<Type::COUNT>& set) noexcept
    : m_bitset(set) {}

  template<typename T>
  EnumSet<T>::operator const std::bitset<T::COUNT>& () const {
    return m_bitset;
  }

  template<typename T>
  EnumSet<T>::operator std::bitset<T::COUNT>& () {
    return m_bitset;
  }

  template<typename T>
  bool EnumSet<T>::test(Type value) const {
    if(value == Type::NONE) {
      return true;
    }
    return m_bitset.test(static_cast<int>(value));
  }

  template<typename T>
  EnumSet<T>& EnumSet<T>::set(const EnumSet& values) {
    m_bitset |= values.m_bitset;
    return *this;
  }

  template<typename T>
  EnumSet<T>& EnumSet<T>::reset(const EnumSet& values) {
    m_bitset &= ~values.m_bitset;
    return *this;
  }

  template<typename T>
  const std::bitset<T::COUNT>& EnumSet<T>::get_bitset() const {
    return m_bitset;
  }

  template<typename T>
  constexpr auto is_structure<EnumSet<T>> = false;

  template<typename T>
  struct Send<EnumSet<T>> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const EnumSet<T>& value) const {
      sender.send(name, value.get_bitset());
    }
  };

  template<typename T>
  struct Receive<EnumSet<T>> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name, EnumSet<T>& value) const {
      value = EnumSet<T>(
        receive<std::bitset<EnumSet<T>::Type::COUNT>>(receiver, name));
    }
  };
}

#endif
