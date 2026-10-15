#ifndef BEAM_SNAPSHOT_LIMIT_HPP
#define BEAM_SNAPSHOT_LIMIT_HPP
#include <algorithm>
#include <format>
#include <limits>
#include <ostream>
#include <string_view>
#include <boost/throw_exception.hpp>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/SerializationException.hpp"

namespace Beam {

  /**
   * Specifies a limit on the number of items to return in a Query's snapshot.
   */
  class SnapshotLimit {
    public:

      /** Whether the limit begins from the head or the tail. */
      enum class Type : int {

        /** The limit is from the beginning towards the end of the data. */
        HEAD,

        /** The limit is from the end towards the beginning of the data. */
        TAIL
      };

      /**
       * Returns a SnapshotLimit specifying no snapshot items are to be
       * returned.
       */
      static const SnapshotLimit NONE;

      /**
       * Returns a SnapshotLimit specifying all snapshot items are to be
       * returned.
       */
      static const SnapshotLimit UNLIMITED;

      /**
       * Returns a SnapshotLimit from the head of the data set.
       * @param size The size of the limit.
       */
      static constexpr SnapshotLimit from_head(int size) noexcept;

      /**
       * Returns a SnapshotLimit from the tail of the data set.
       * @param size The size of the limit.
       */
      static constexpr SnapshotLimit from_tail(int size) noexcept;

      /** Constructs a SnapshotLimit with a size of 0. */
      constexpr SnapshotLimit() noexcept;

      /**
       * Constructs a SnapshotLimit.
       * @param type The Type of limit.
       * @param size The size of the limit.
       */
      constexpr SnapshotLimit(Type type, int size) noexcept;

      /** Returns the Type of limit. */
      constexpr Type get_type() const noexcept;

      /** Returns the size of the limit. */
      constexpr int get_size() const noexcept;

      constexpr bool operator ==(const SnapshotLimit&) const noexcept;

    private:
      friend struct Shuttle<SnapshotLimit>;
      Type m_type;
      int m_size;
  };

  inline const SnapshotLimit SnapshotLimit::NONE =
    SnapshotLimit(SnapshotLimit::Type::HEAD, 0);

  inline const SnapshotLimit SnapshotLimit::UNLIMITED =
    SnapshotLimit(SnapshotLimit::Type::HEAD, std::numeric_limits<int>::max());

  constexpr SnapshotLimit SnapshotLimit::from_head(int size) noexcept {
    return SnapshotLimit(SnapshotLimit::Type::HEAD, size);
  }

  constexpr SnapshotLimit SnapshotLimit::from_tail(int size) noexcept {
    return SnapshotLimit(SnapshotLimit::Type::TAIL, size);
  }

  constexpr SnapshotLimit::SnapshotLimit() noexcept
    : m_type(Type::HEAD),
      m_size(0) {}

  constexpr SnapshotLimit::SnapshotLimit(Type type, int size) noexcept
    : m_type(size <= 0 || size == std::numeric_limits<int>::max() ? Type::HEAD :
        type),
      m_size(std::max(0, size)) {}

  constexpr bool SnapshotLimit::operator ==(
      const SnapshotLimit& rhs) const noexcept {
    if(m_size == 0) {
      return rhs.m_size == 0;
    }
    if(m_size == std::numeric_limits<int>::max()) {
      return rhs.m_size == std::numeric_limits<int>::max();
    }
    return m_type == rhs.m_type && m_size == rhs.m_size;
  }

  constexpr SnapshotLimit::Type SnapshotLimit::get_type() const noexcept {
    return m_type;
  }

  constexpr int SnapshotLimit::get_size() const noexcept {
    return m_size;
  }

  template<>
  struct Shuttle<Beam::SnapshotLimit> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, SnapshotLimit& value, unsigned int version) const {
      shuttle.shuttle("type", value.m_type);
      shuttle.shuttle("size", value.m_size);
      if(IsReceiver<S>) {
        if(value.m_size == 0 ||
            value.m_size == std::numeric_limits<int>::max()) {
          value.m_type = SnapshotLimit::Type::HEAD;
        }
        if(value.m_size < 0) {
          value.m_type = SnapshotLimit::Type::HEAD;
          value.m_size = 0;
          boost::throw_with_location(SerializationException("Invalid size."));
        }
        if(value.m_type != SnapshotLimit::Type::HEAD &&
            value.m_type != SnapshotLimit::Type::TAIL) {
          value.m_type = SnapshotLimit::Type::HEAD;
          value.m_size = 0;
          boost::throw_with_location(SerializationException("Invalid type."));
        }
      }
    }
  };
}

template<>
class std::formatter<Beam::SnapshotLimit::Type> :
    public std::formatter<std::string_view> {
  public:
    auto format(Beam::SnapshotLimit::Type value, auto& context) const {
      auto name = [&] {
        if(value == Beam::SnapshotLimit::Type::HEAD) {
          return std::string_view("HEAD");
        } else if(value == Beam::SnapshotLimit::Type::TAIL) {
          return std::string_view("TAIL");
        }
        return std::string_view("NONE");
      }();
      return std::formatter<std::string_view>::format(name, context);
    }
};

template<>
class std::formatter<Beam::SnapshotLimit> :
    public std::formatter<std::string_view> {
  public:
    auto format(Beam::SnapshotLimit value, auto& context) const {
      if(value == Beam::SnapshotLimit::NONE) {
        return std::formatter<std::string_view>::format("None", context);
      } else if(value == Beam::SnapshotLimit::UNLIMITED) {
        return std::formatter<std::string_view>::format("Unlimited", context);
      }
      return std::formatter<std::string_view>::format(
        std::format("({} {})", value.get_type(), value.get_size()), context);
    }
};

namespace Beam {
  inline std::ostream& operator <<(
      std::ostream& out, SnapshotLimit::Type type) {
    return out << std::format("{}", type);
  }

  inline std::ostream& operator <<(std::ostream& out, SnapshotLimit limit) {
    return out << std::format("{}", limit);
  }
}

#endif
