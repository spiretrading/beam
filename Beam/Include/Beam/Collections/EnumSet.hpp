#ifndef BEAM_ENUM_SET_HPP
#define BEAM_ENUM_SET_HPP
#include <bitset>
#include <type_traits>
#include "Beam/Collections/Collections.hpp"
#include "Beam/Collections/Enum.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/ShuttleBitset.hpp"

namespace Beam {

  /**
   * Stores a set of enum values.
   * @param <T> The enum type to store.
   */
  template<typename T>
  class EnumSet {
    public:

      /** The enum type stored. */
      using Type = T;

      /**
       * Builds an EnumSet from its underlying representation.
       * @param value The value to build the EnumSet from.
       */
      static EnumSet FromRepresentation(std::uint32_t value);

      /** Constructs an empty EnumSet. */
      EnumSet() = default;

      /**
       * Constructs an EnumSet containing a single value.
       * @param value The value to store.
       */
      EnumSet(Type value);

      /**
       * Constructs an EnumSet containing a single value.
       * @param value The value to store.
       */
      EnumSet(typename Type::Type value);

      /**
       * Constructs an EnumSet from a bitset.
       * @param set The bitset to represent.
       */
      explicit EnumSet(const std::bitset<Type::COUNT>& set);

      /** Converts this set to a bitset. */
      explicit operator const std::bitset<T::COUNT>& () const;

      /** Converts this set to a bitset. */
      explicit operator std::bitset<T::COUNT>& ();

      /**
       * Tests two EnumSet's for equality.
       * @param rhs The right hand side to test.
       * @return <code>true</code> iff this set is equal to <i>rhs</i>.
       */
      bool operator ==(const EnumSet& rhs) const;

      /**
       * Tests two EnumSet's for inequality.
       * @param rhs The right hand side to test.
       * @return <code>true</code> iff this set is not equal to <i>rhs</i>.
       */
      bool operator !=(const EnumSet& rhs) const;

      /**
       * Tests if a value belongs to this set.
       * @param value The value to test.
       * @return <code>true</code> iff the <i>value</i> is a member of this set.
       */
      bool Test(Type value) const;

      /**
       * Adds all values in a set to this set.
       * @param set The set of values to add to this set.
       */
      EnumSet& SetAll(const EnumSet& values);

      /**
       * Adds a value to this set.
       * @param value The value to add to this set.
       */
      EnumSet& Set(Type value);

      /**
       * Removes a value from this set.
       * @param value The value to remove from this set.
       */
      EnumSet& Unset(Type value);

      /** Returns the bitset. */
      const std::bitset<T::COUNT>& GetBitset() const;

    private:
      std::bitset<Type::COUNT> m_bitset;
  };

  template<typename T>
  EnumSet<T> operator &(const EnumSet<T>& lhs, const EnumSet<T>& rhs) {
    return EnumSet<T>(lhs.GetBitset() & rhs.GetBitset());
  }

  template<typename T>
  EnumSet<T> operator |(const EnumSet<T>& lhs, const EnumSet<T>& rhs) {
    return EnumSet<T>(lhs.GetBitset() | rhs.GetBitset());
  }

  template<typename T>
  EnumSet<T> operator ^(const EnumSet<T>& lhs, const EnumSet<T>& rhs) {
    return EnumSet<T>(lhs.GetBitset() ^ rhs.GetBitset());
  }

  template<typename T>
  EnumSet<T> EnumSet<T>::FromRepresentation(std::uint32_t value) {
    auto set = EnumSet();
    set.m_bitset = value;
    return set;
  }

  template<typename T>
  EnumSet<T>::EnumSet(Type value) {
    Set(value);
  }

  template<typename T>
  EnumSet<T>::EnumSet(typename Type::Type value) {
    Set(value);
  }

  template<typename T>
  EnumSet<T>::EnumSet(const std::bitset<Type::COUNT>& set)
    : m_bitset{set} {}

  template<typename T>
  EnumSet<T>::operator const std::bitset<T::COUNT>& () const {
    return m_bitset;
  }

  template<typename T>
  EnumSet<T>::operator std::bitset<T::COUNT>& () {
    return m_bitset;
  }

  template<typename T>
  bool EnumSet<T>::operator ==(const EnumSet& rhs) const {
    return m_bitset == rhs.m_bitset;
  }

  template<typename T>
  bool EnumSet<T>::operator !=(const EnumSet& rhs) const {
    return !(*this == rhs);
  }

  template<typename T>
  bool EnumSet<T>::Test(Type value) const {
    if(value == Type::NONE) {
      return true;
    }
    return m_bitset.test(static_cast<int>(value));
  }

  template<typename T>
  EnumSet<T>& EnumSet<T>::SetAll(const EnumSet& values) {
    m_bitset |= values.m_bitset;
    return *this;
  }

  template<typename T>
  EnumSet<T>& EnumSet<T>::Set(Type value) {
    if(value != Type::NONE) {
      m_bitset.set(static_cast<int>(value));
    }
    return *this;
  }

  template<typename T>
  EnumSet<T>& EnumSet<T>::Unset(Type value) {
    if(value != Type::NONE) {
      m_bitset.reset(static_cast<int>(value));
    }
    return *this;
  }

  template<typename T>
  const std::bitset<T::COUNT>& EnumSet<T>::GetBitset() const {
    return m_bitset;
  }
}

namespace Beam::Serialization {
  template<typename T>
  struct IsStructure<EnumSet<T>> : std::false_type {};

  template<typename T>
  struct Send<EnumSet<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const EnumSet<T>& value) const {
      shuttle.Send(name, value.GetBitset());
    }
  };

  template<typename T>
  struct Receive<EnumSet<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        EnumSet<T>& value) const {
      auto set = std::bitset<EnumSet<T>::Type::COUNT>();
      shuttle.Shuttle(name, set);
      value = EnumSet<T>{set};
    }
  };
}

#endif
