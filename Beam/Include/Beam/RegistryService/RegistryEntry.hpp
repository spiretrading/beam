#ifndef BEAM_REGISTRYENTRY_HPP
#define BEAM_REGISTRYENTRY_HPP
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include "Beam/Collections/Enum.hpp"
#include "Beam/RegistryService/RegistryService.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam::RegistryService {
namespace Details {
  BEAM_ENUM(RegistryEntryType,

    /** Represents a registry directory. */
    DIRECTORY,

    /** Represents a registry value. */
    VALUE);
}

  /** Represents a single entry within the registry. */
  struct RegistryEntry {

    /** Enumerates types of RegistryEntries. */
    using Type = Details::RegistryEntryType;

    /** The Type. */
    Type m_type;

    /** The unique id. */
    std::uint64_t m_id;

    /** The name. */
    std::string m_name;

    /** The version. */
    std::uint64_t m_version;

    /** Returns the root registry directory. */
    static const RegistryEntry& GetRoot();

    /** Constructs an empty RegistryEntry. */
    RegistryEntry();

    /**
     * Constructs a RegistryEntry.
     * @param type The Type.
     * @param id The id.
     * @param name The name.
     * @param version The entry's version.
     */
    RegistryEntry(Type type, std::uint64_t id, std::string name,
      std::uint64_t version);

    /**
     * Tests if this RegistryEntry is less than another.
     * @param rhs The right-hand side of the operation.
     * @return <code>true</code> iff this RegistryEntry's id is less than
     *         <i>rhs</i>'s id.
     */
    bool operator <(const RegistryEntry& rhs) const;

    /**
     * Tests if this RegistryEntry identifies the same RegistryEntry as
     * another.
     * @param rhs The right-hand side of the operation.
     * @return <code>true</code> iff this RegistryEntry identifies the same
     *         RegistryEntry as <i>rhs</i>.
     */
    bool operator ==(const RegistryEntry& rhs) const;

    /**
     * Tests if this RegistryEntry identifies a different RegistryEntry as
     * another.
     * @param rhs The right-hand side of the operation.
     * @return <code>true</code> iff this RegistryEntry does not identify the
     *         same RegistryEntry as <i>rhs</i>.
     */
    bool operator !=(const RegistryEntry& rhs) const;
  };

  inline std::size_t hash_value(const RegistryEntry& value) {
    return static_cast<std::size_t>(value.m_id);
  }

  inline const RegistryEntry& RegistryEntry::GetRoot() {
    static auto rootDirectory = RegistryEntry(Type::DIRECTORY, 0, "", 0);
    return rootDirectory;
  }

  inline RegistryEntry::RegistryEntry()
    : m_id(static_cast<std::uint64_t>(-1)),
      m_version(0) {}

  inline RegistryEntry::RegistryEntry(Type type, std::uint64_t id,
    std::string name, std::uint64_t version)
    : m_type(type),
      m_id(id),
      m_name(std::move(name)),
      m_version(version) {}

  inline bool RegistryEntry::operator <(const RegistryEntry& rhs) const {
    return m_id < rhs.m_id;
  }

  inline bool RegistryEntry::operator ==(const RegistryEntry& rhs) const {
    return m_id == rhs.m_id && m_type == rhs.m_type;
  }

  inline bool RegistryEntry::operator !=(const RegistryEntry& rhs) const {
    return !(*this == rhs);
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<RegistryService::RegistryEntry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, RegistryService::RegistryEntry& value,
        unsigned int version) const {
      shuttle.Shuttle("type", value.m_type);
      shuttle.Shuttle("id", value.m_id);
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("version", value.m_version);
    }
  };
}

namespace std {
  template <>
  struct hash<Beam::RegistryService::RegistryEntry> {
    size_t operator()(const Beam::RegistryService::RegistryEntry& value) const {
      return hash_value(value);
    }
  };
}

#endif
