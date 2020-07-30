#ifndef BEAM_ENUM_HPP
#define BEAM_ENUM_HPP
#include <cstdint>
#include <type_traits>
#include "Beam/Collections/Collections.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Utilities/Preprocessor.hpp"

#define BEAM_ENUM(name, ...)                                                   \
  struct name##EnumMembers {                                                   \
    enum Type : int {                                                          \
      NONE = -1,                                                               \
      __VA_ARGS__                                                              \
    };                                                                         \
  };                                                                           \
                                                                               \
  using name = ::Beam::Enum<name##EnumMembers, PP_NARG(__VA_ARGS__)>;

namespace Beam {

  /**
   * Wraps an enum value.
   * @param <T> The enum type to represent.
   * @param <N> The number of members enumerated.
   */
  template<typename T, std::size_t N>
  class Enum : public T {
    public:

      /** The enum type represented. */
      using Type = typename T::Type;

      /** The number of members enumerated. */
      static constexpr auto COUNT = N;

      /** Constructs an Enum with a value of NONE. */
      Enum();

      /**
       * Constructs an Enum.
       * @param value The value to represent.
       */
      Enum(Type value);

      /**
       * Constructs an Enum.
       * @param value The value to represent.
       */
      explicit Enum(int value);

      /** Converts this Enum into its underlying type. */
      operator Type () const;

      /**
       * Tests if this Enum comes before another.
       * @param rhs The Enum to test against.
       * @return <code>true</code> iff this Enum comes before <i>rhs</i>.
       */
      bool operator <(Enum rhs) const;

      /**
       * Tests if this Enum comes before another.
       * @param rhs The Enum to test against.
       * @return <code>true</code> iff this Enum comes before <i>rhs</i>.
       */
      bool operator <(Type rhs) const;

      /**
       * Tests two Enums for equality.
       * @param rhs The right hand side of the equality.
       * @return <code>true</code> iff this is equal to <i>rhs</i>.
       */
      bool operator ==(Enum rhs) const;

      /**
       * Tests two Enums for inequality.
       * @param rhs The right hand side of the inequality.
       * @return <code>true</code> iff this is not equal to <i>rhs</i>.
       */
      bool operator !=(Enum rhs) const;

      /**
       * Tests two Enums for equality.
       * @param rhs The right hand side of the equality.
       * @return <code>true</code> iff this is equal to <i>rhs</i>.
       */
      bool operator ==(Type rhs) const;

      /**
       * Tests two Enums for inequality.
       * @param rhs The right hand side of the inequality.
       * @return <code>true</code> iff this is not equal to <i>rhs</i>.
       */
      bool operator !=(Type rhs) const;

    private:
      Type m_value;
  };

  template<typename T, std::size_t N>
  Enum<T, N>::Enum()
    : m_value(Type::NONE) {}

  template<typename T, std::size_t N>
  Enum<T, N>::Enum(Type value)
    : m_value(value) {}

  template<typename T, std::size_t N>
  Enum<T, N>::Enum(int value)
    : m_value(static_cast<Type>(value)) {}

  template<typename T, std::size_t N>
  Enum<T, N>::operator Type () const {
    return m_value;
  }

  template<typename T, std::size_t N>
  bool Enum<T, N>::operator <(Enum rhs) const {
    return m_value < rhs.m_value;
  }

  template<typename T, std::size_t N>
  bool Enum<T, N>::operator <(Type rhs) const {
    return m_value < rhs;
  }

  template<typename T, std::size_t N>
  bool Enum<T, N>::operator ==(Enum rhs) const {
    return m_value == rhs.m_value;
  }

  template<typename T, std::size_t N>
  bool Enum<T, N>::operator !=(Enum rhs) const {
    return !(*this == rhs);
  }

  template<typename T, std::size_t N>
  bool Enum<T, N>::operator ==(Type rhs) const {
    return m_value == rhs;
  }

  template<typename T, std::size_t N>
  bool Enum<T, N>::operator !=(Type rhs) const {
    return !(*this == rhs);
  }

  template<typename T, std::size_t N>
  std::size_t hash_value(Enum<T, N> value) {
    return static_cast<std::size_t>(value);
  }
}

namespace std {
  template<typename T, std::size_t N>
  struct hash<Beam::Enum<T, N>> {
    std::size_t operator ()(Beam::Enum<T, N> value) const {
      return Beam::hash_value(value);
    }
  };
}

namespace Beam::Serialization {
  template<typename T, std::size_t N>
  struct IsStructure<Enum<T, N>> : std::false_type {};

  template<typename T, std::size_t N>
  struct Send<Enum<T, N>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const Enum<T, N>& value) const {
      shuttle.Send(name, static_cast<typename Enum<T, N>::Type>(value));
    }
  };

  template<typename T, std::size_t N>
  struct Receive<Enum<T, N>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Enum<T, N>& value) const {
      auto e = typename Enum<T, N>::Type();
      shuttle.Shuttle(name, e);
      value = e;
    }
  };
}

#endif
