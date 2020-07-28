#ifndef BEAM_DIRECTORY_ENTRY_HPP
#define BEAM_DIRECTORY_ENTRY_HPP
#include <ostream>
#include <string>
#include <tuple>
#include "Beam/Collections/Enum.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {
namespace Details {
  BEAM_ENUM(DirectoryEntryType,

    /** Account. */
    ACCOUNT,

    /** Directory. */
    DIRECTORY);
}

  /** Represents an entry that can be stored within a directory. */
  struct DirectoryEntry {

    /** Enumerates types of DirectoryEntries. */
    using Type = Details::DirectoryEntryType;

    /** The Type. */
    Type m_type;

    /** The unique id. */
    unsigned int m_id;

    /** The name. */
    std::string m_name;

    /** Returns the DirectoryEntry representing the root account. */
    static const DirectoryEntry& GetRootAccount();

    /** Returns the DirectoryEntry representing the star directory. */
    static const DirectoryEntry& GetStarDirectory();

    /**
     * Compares two DirectoryEntry's by name.
     * @param lhs The left hand side of the comparison.
     * @param rhs The right hand side of the comparison.
     * @return <code>true</code> iff <i>lhs</i>'s name preceeds <i>rhs</i>.
     */
    static bool NameComparator(const DirectoryEntry& lhs,
      const DirectoryEntry& rhs);

    /**
     * Makes a DirectoryEntry representing an account.
     * @param id The id.
     * @param name The name.
     * @return A DirectoryEntry representing an account.
     */
    static DirectoryEntry MakeAccount(unsigned int id, std::string name);

    /**
     * Makes a DirectoryEntry representing an account.
     * @param id The id.
     * @return An unnamed DirectoryEntry representing an account.
     */
    static DirectoryEntry MakeAccount(unsigned int id);

    /**
     * Makes a DirectoryEntry representing a directory.
     * @param id The id.
     * @param name The name.
     * @return A DirectoryEntry representing a directory.
     */
    static DirectoryEntry MakeDirectory(unsigned int id, std::string name);

    /**
     * Makes a DirectoryEntry representing a directory.
     * @param id The id.
     * @return An unnamed DirectoryEntry representing a directory.
     */
    static DirectoryEntry MakeDirectory(unsigned int id);

    /** Constructs an empty DirectoryEntry. */
    DirectoryEntry();

    /**
     * Constructs a DirectoryEntry.
     * @param type The Type.
     * @param id The id.
     * @param name The name.
     */
    DirectoryEntry(Type type, unsigned int id, std::string name);

    /**
     * Tests if this DirectoryEntry's id is less than another.
     * @param rhs The right-hand side of the operation.
     * @return <code>true</code> iff this DirectoryEntry's id is less than
     *         <i>rhs</i>'s id.
     */
    bool operator <(const DirectoryEntry& rhs) const;

    /**
     * Tests if this DirectoryEntry identifies the same DirectoryEntry as
     * another.
     * @param rhs The right-hand side of the operation.
     * @return <code>true</code> iff this DirectoryEntry identifies the same
     *         DirectoryEntry as <i>rhs</i>.
     */
    bool operator ==(const DirectoryEntry& rhs) const;

    /**
     * Tests if this DirectoryEntry identifies a different DirectoryEntry as
     * another.
     * @param rhs The right-hand side of the operation.
     * @return <code>true</code> iff this DirectoryEntry does not identify the
     *         same DirectoryEntry as <i>rhs</i>.
     */
    bool operator !=(const DirectoryEntry& rhs) const;
  };

  inline std::ostream& operator <<(std::ostream& out,
      DirectoryEntry::Type type) {
    if(type == DirectoryEntry::Type::ACCOUNT) {
      return out << "ACCOUNT";
    } else if(type == DirectoryEntry::Type::DIRECTORY) {
      return out << "DIRECTORY";
    } else {
      return out << "NONE";
    }
  }

  inline std::ostream& operator <<(std::ostream& out,
      const DirectoryEntry& entry) {
    if(entry.m_type == DirectoryEntry::Type::NONE || entry.m_id == -1) {
      return out << "NONE";
    }
    out << "(" << entry.m_type << " " << entry.m_id;
    if(!entry.m_name.empty()) {
      out << " " << entry.m_name;
    }
    return out << ")";
  }

  inline std::size_t hash_value(const DirectoryEntry& value) {
    return value.m_id;
  }

  inline const DirectoryEntry& DirectoryEntry::GetRootAccount() {
    static const auto rootAccount = DirectoryEntry::MakeAccount(1, "root");
    return rootAccount;
  }

  inline const DirectoryEntry& DirectoryEntry::GetStarDirectory() {
    static const auto starDirectory = DirectoryEntry::MakeDirectory(0, "*");
    return starDirectory;
  }

  inline bool DirectoryEntry::NameComparator(const DirectoryEntry& lhs,
      const DirectoryEntry& rhs) {
    return std::tie(lhs.m_name, lhs.m_id) < std::tie(rhs.m_name, rhs.m_id);
  }

  inline DirectoryEntry DirectoryEntry::MakeAccount(unsigned int id,
      std::string name) {
    return DirectoryEntry{Type::ACCOUNT, id, std::move(name)};
  }

  inline DirectoryEntry DirectoryEntry::MakeAccount(unsigned int id) {
    return MakeAccount(id, {});
  }

  inline DirectoryEntry DirectoryEntry::MakeDirectory(unsigned int id,
      std::string name) {
    return DirectoryEntry(Type::DIRECTORY, id, std::move(name));
  }

  inline DirectoryEntry DirectoryEntry::MakeDirectory(unsigned int id) {
    return MakeDirectory(id, {});
  }

  inline DirectoryEntry::DirectoryEntry()
    : m_type(Type::NONE),
      m_id(-1) {}

  inline DirectoryEntry::DirectoryEntry(Type type, unsigned int id,
    std::string name)
    : m_type(type),
      m_id(id),
      m_name(std::move(name)) {}

  inline bool DirectoryEntry::operator <(const DirectoryEntry& rhs) const {
    return m_id < rhs.m_id;
  }

  inline bool DirectoryEntry::operator ==(const DirectoryEntry& rhs) const {
    return m_id == rhs.m_id && m_type == rhs.m_type;
  }

  inline bool DirectoryEntry::operator !=(const DirectoryEntry& rhs) const {
    return !(*this == rhs);
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<ServiceLocator::DirectoryEntry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, ServiceLocator::DirectoryEntry& value,
        unsigned int version) const {
      shuttle.Shuttle("type", value.m_type);
      shuttle.Shuttle("id", value.m_id);
      shuttle.Shuttle("name", value.m_name);
    }
  };
}

namespace std {
  template <>
  struct hash<Beam::ServiceLocator::DirectoryEntry> {
    size_t operator()(const Beam::ServiceLocator::DirectoryEntry& value) const {
      return hash_value(value);
    }
  };
}

#endif
