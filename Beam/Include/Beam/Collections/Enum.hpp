#ifndef BEAM_ENUM_HPP
#define BEAM_ENUM_HPP
#include <array>
#include <cstdint>
#include <format>
#include <ostream>
#include <string_view>
#include <type_traits>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/variadic/size.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Utilities/ToString.hpp"

#define BEAM_ENUM_STRINGIZE_EACH(r, data, elem) BOOST_PP_STRINGIZE(elem)

#define BEAM_BASIC_ENUM(name, ...)                                             \
  struct BOOST_PP_CAT(name, EnumMembers) {                                     \
    enum Type : int {                                                          \
      NONE = -1,                                                               \
      __VA_ARGS__                                                              \
    };                                                                         \
                                                                               \
    friend auto get_enum_members(Type) {                                       \
      return ::std::type_identity<BOOST_PP_CAT(name, EnumMembers)>();          \
    }                                                                          \
                                                                               \
    auto operator <=>(const BOOST_PP_CAT(name, EnumMembers)& rhs) const =      \
      default;                                                                 \
  };                                                                           \
                                                                               \
  using name = ::Beam::Enum<BOOST_PP_CAT(name, EnumMembers),                   \
    BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)>;

#define BEAM_ENUM(name, ...)                                                   \
  struct BOOST_PP_CAT(name, EnumMembers) {                                     \
    enum Type : int {                                                          \
      NONE = -1,                                                               \
      __VA_ARGS__                                                              \
    };                                                                         \
                                                                               \
    friend auto get_enum_members(Type) {                                       \
      return ::std::type_identity<BOOST_PP_CAT(name, EnumMembers)>();           \
    }                                                                          \
                                                                               \
    auto operator <=>(const BOOST_PP_CAT(name, EnumMembers)& rhs) const =      \
      default;                                                                 \
                                                                               \
    static constexpr auto NAMES =                                              \
      std::array<std::string_view, BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)>{       \
        BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(                              \
          BEAM_ENUM_STRINGIZE_EACH, _,                                         \
          BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))};                            \
                                                                               \
    static constexpr Type from(std::string_view source) noexcept {             \
      for(auto i = std::size_t(0); i < NAMES.size(); ++i) {                    \
        if(source == NAMES[i]) {                                               \
          return static_cast<Type>(static_cast<int>(i));                       \
        }                                                                      \
      }                                                                        \
      return Type::NONE;                                                       \
    }                                                                          \
  };                                                                           \
                                                                               \
  using name = ::Beam::Enum<BOOST_PP_CAT(name, EnumMembers),                   \
    BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)>;                                      \
                                                                               \
  inline std::ostream& operator <<(                                            \
      std::ostream& out, BOOST_PP_CAT(name, EnumMembers)::Type value) {        \
    return out << ::std::format(out.getloc(), "{}", value);                    \
  }

namespace Beam {

  /**
   * Wraps an enum value.
   * @tparam T The enum type to represent.
   * @tparam N The number of members enumerated.
   */
  template<typename T, std::size_t N>
  class Enum : public T {
    public:

      /** The enum type represented. */
      using Type = typename T::Type;

      /** The number of members enumerated. */
      static constexpr auto COUNT = N;

      /**
       * Converts a string into an Enum.
       * @param source The string to convert.
       * @return The Enum represented by <i>source</i>.
       */
      static constexpr auto from(std::string_view source) noexcept {
        return T::from(source);
      }

      /** Constructs an Enum with a value of NONE. */
      Enum() noexcept;

      /**
       * Constructs an Enum.
       * @param value The value to represent.
       */
      Enum(Type value) noexcept;

      /**
       * Constructs an Enum.
       * @param value The value to represent.
       */
      explicit Enum(int value) noexcept;

      /** Converts this Enum into its underlying type. */
      operator Type () const;

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
      bool operator ==(Type rhs) const;

      /**
       * Tests two Enums for inequality.
       * @param rhs The right hand side of the inequality.
       * @return <code>true</code> iff this is not equal to <i>rhs</i>.
       */
      bool operator !=(Type rhs) const;

      auto operator <=>(const Enum& rhs) const = default;

    private:
      Type m_value;
  };

  template<typename T, std::size_t N> requires requires {
    { std::declval<std::ostream&>() <<
        std::declval<const typename T::Type&>()
    } -> std::same_as<std::ostream&>;
  }
  std::ostream& operator <<(std::ostream& out, Enum<T, N> value) {
    return out << static_cast<typename Enum<T, N>::Type>(value);
  }

  template<typename T, std::size_t N>
  Enum<T, N>::Enum() noexcept
    : m_value(Type::NONE) {}

  template<typename T, std::size_t N>
  Enum<T, N>::Enum(Type value) noexcept
    : m_value(value) {}

  template<typename T, std::size_t N>
  Enum<T, N>::Enum(int value) noexcept
    : m_value(static_cast<Type>(value)) {}

  template<typename T, std::size_t N>
  Enum<T, N>::operator Type () const {
    return m_value;
  }

  template<typename T, std::size_t N>
  bool Enum<T, N>::operator <(Type rhs) const {
    return m_value < rhs;
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

  template<typename T, std::size_t N>
  constexpr auto is_structure<Enum<T, N>> = false;

  template<typename T, std::size_t N>
  struct Send<Enum<T, N>> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const Enum<T, N>& value) const {
      sender.send(name, static_cast<typename Enum<T, N>::Type>(value));
    }
  };

  template<typename T, std::size_t N>
  struct Receive<Enum<T, N>> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name, Enum<T, N>& value) const {
      auto e = typename Enum<T, N>::Type();
      receiver.receive(name, e);
      value = e;
    }
  };
}

namespace std {
  template<typename T, std::size_t N>
  struct hash<Beam::Enum<T, N>> {
    std::size_t operator ()(Beam::Enum<T, N> value) const noexcept {
      return Beam::hash_value(value);
    }
  };
}

namespace Beam::Details {
  template<typename T>
  concept IsGeneratedEnum = std::is_enum_v<T> && requires(T value) {
    typename decltype(get_enum_members(value))::type;
  };
}

template<Beam::Details::IsGeneratedEnum T>
class std::formatter<T> : public std::formatter<std::string_view> {
  public:
    auto format(T value, auto& context) const {
      using Members = typename decltype(get_enum_members(value))::type;
      if constexpr(requires { Members::NAMES; }) {
        auto name = [&] {
          auto index = static_cast<std::size_t>(value);
          if(index < Members::NAMES.size()) {
            return Members::NAMES[index];
          }
          return std::string_view("NONE");
        }();
        return std::formatter<std::string_view>::format(name, context);
      } else {
        auto output = std::ostringstream();
        output.imbue(context.locale());
        output << value;
        return std::formatter<std::string_view>::format(output.view(), context);
      }
    }
};

template<typename T, std::size_t N> requires
  std::formattable<typename T::Type, char>
class std::formatter<Beam::Enum<T, N>> :
    public std::formatter<typename T::Type> {
  public:
    auto format(Beam::Enum<T, N> value, auto& context) const {
      return std::formatter<typename T::Type>::format(
        static_cast<typename T::Type>(value), context);
    }
};

#endif
