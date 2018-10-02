#ifndef BEAM_SNAPSHOT_LIMIT_HPP
#define BEAM_SNAPSHOT_LIMIT_HPP
#include <algorithm>
#include <limits>
#include <ostream>
#include <boost/throw_exception.hpp>
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/SerializationException.hpp"

namespace Beam::Queries {

  /** Specifies a limit on the number of items to return in a Query's snapshot.
   */
  class SnapshotLimit {
    public:

      /*! \enum Type
          \brief Whether the limit begins from the head or the tail.
       */
      enum class Type : int {

        //! The limit is from the beginning towards the end of the data.
        HEAD,

        //! The limit is from the end towards the beginning of the data.
        TAIL
      };

      //! Returns a SnapshotLimit specifying no snapshot items are to be
      //! returned.
      static constexpr SnapshotLimit None();

      //! Returns a SnapshotLimit specifying all snapshot items are to be
      //! returned.
      static constexpr SnapshotLimit Unlimited();

      //! Returns a SnapshotLimit from the head of the data set.
      /*!
        \param size The size of the limit.
      */
      static constexpr SnapshotLimit FromHead(int size);

      //! Returns a SnapshotLimit from the tail of the data set.
      /*!
        \param size The size of the limit.
      */
      static constexpr SnapshotLimit FromTail(int size);

      //! Constructs a SnapshotLimit with a size of 0.
      constexpr SnapshotLimit();

      //! Constructs a SnapshotLimit.
      /*!
        \param type The Type of limit.
        \param size The size of the limit.
      */
      constexpr SnapshotLimit(Type type, int size);

      //! Returns the Type of limit.
      constexpr Type GetType() const;

      //! Returns the size of the limit.
      constexpr int GetSize() const;

      //! Tests if two SnapshotLimits are equal.
      constexpr bool operator ==(const SnapshotLimit& rhs) const;

      //! Tests if two SnapshotLimit are not equal.
      constexpr bool operator !=(const SnapshotLimit& rhs) const;

    private:
      friend struct Serialization::Shuttle<SnapshotLimit>;
      Type m_type;
      int m_size;
  };

  inline std::ostream& operator <<(std::ostream& out,
      SnapshotLimit::Type type) {
    if(type == SnapshotLimit::Type::HEAD) {
      return out << "HEAD";
    } else if(type == SnapshotLimit::Type::TAIL) {
      return out << "TAIL";
    }
    return out << "NONE";
  }

  inline std::ostream& operator <<(std::ostream& out,
      const SnapshotLimit& limit) {
    if(limit == SnapshotLimit::None()) {
      return out << "None";
    } else if(limit == SnapshotLimit::Unlimited()) {
      return out << "Unlimited";
    }
    return out << "(" << limit.GetType() << " " << limit.GetSize() << ")";
  }

  constexpr SnapshotLimit SnapshotLimit::None() {
    return SnapshotLimit(SnapshotLimit::Type::HEAD, 0);
  }

  constexpr SnapshotLimit SnapshotLimit::Unlimited() {
    return SnapshotLimit(SnapshotLimit::Type::HEAD,
      std::numeric_limits<int>::max());
  }

  constexpr SnapshotLimit SnapshotLimit::FromHead(int size) {
    return SnapshotLimit(SnapshotLimit::Type::HEAD, size);
  }

  constexpr SnapshotLimit SnapshotLimit::FromTail(int size) {
    return SnapshotLimit(SnapshotLimit::Type::TAIL, size);
  }

  constexpr SnapshotLimit::SnapshotLimit()
      : m_type(Type::HEAD),
        m_size(0) {}

  constexpr SnapshotLimit::SnapshotLimit(Type type, int size)
      : m_type(size <= 0 || size == std::numeric_limits<int>::max() ?
          Type::HEAD : type),
        m_size(std::max(0, size)) {}

  constexpr bool SnapshotLimit::operator ==(const SnapshotLimit& rhs) const {
    if(m_size == 0) {
      return rhs.m_size == 0;
    }
    if(m_size == std::numeric_limits<int>::max()) {
      return rhs.m_size == std::numeric_limits<int>::max();
    }
    return m_type == rhs.m_type && m_size == rhs.m_size;
  }

  constexpr bool SnapshotLimit::operator !=(const SnapshotLimit& rhs) const {
    return !(*this == rhs);
  }

  constexpr SnapshotLimit::Type SnapshotLimit::GetType() const {
    return m_type;
  }

  constexpr int SnapshotLimit::GetSize() const {
    return m_size;
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Beam::Queries::SnapshotLimit> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Queries::SnapshotLimit& value,
        unsigned int version) {
      shuttle.Shuttle("type", value.m_type);
      shuttle.Shuttle("size", value.m_size);
      if(IsReceiver<Shuttler>::value) {
        if(value.m_size == 0 ||
            value.m_size == std::numeric_limits<int>::max()) {
          value.m_type = Queries::SnapshotLimit::Type::HEAD;
        }
        if(value.m_size < 0) {
          value.m_type = Queries::SnapshotLimit::Type::HEAD;
          value.m_size = 0;
          BOOST_THROW_EXCEPTION(SerializationException("Invalid size."));
        }
        if(value.m_type != Queries::SnapshotLimit::Type::HEAD &&
            value.m_type != Queries::SnapshotLimit::Type::TAIL) {
          value.m_type = Queries::SnapshotLimit::Type::HEAD;
          value.m_size = 0;
          BOOST_THROW_EXCEPTION(SerializationException("Invalid type."));
        }
      }
    }
  };
}

#endif
