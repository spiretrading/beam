#ifndef BEAM_DIRECTORY_ENTRY_HPP
#define BEAM_DIRECTORY_ENTRY_HPP
#include <ostream>
#include <string>
#include <tuple>
#include "Beam/Collections/Enum.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
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
    static const DirectoryEntry ROOT_ACCOUNT;

    /** Returns the DirectoryEntry representing the star directory. */
    static const DirectoryEntry STAR_DIRECTORY;

    /**
     * Compares two DirectoryEntry's by name.
     * @param lhs The left hand side of the comparison.
     * @param rhs The right hand side of the comparison.
     * @return <code>true</code> iff <i>lhs</i>'s name preceeds <i>rhs</i>.
     */
    static bool name_comparator(
      const DirectoryEntry& lhs, const DirectoryEntry& rhs);

    /**
     * Makes a DirectoryEntry representing an account.
     * @param id The id.
     * @param name The name.
     * @return A DirectoryEntry representing an account.
     */
    static DirectoryEntry make_account(unsigned int id, std::string name);

    /**
     * Makes a DirectoryEntry representing an account.
     * @param id The id.
     * @return An unnamed DirectoryEntry representing an account.
     */
    static DirectoryEntry make_account(unsigned int id);

    /**
     * Makes a DirectoryEntry representing a directory.
     * @param id The id.
     * @param name The name.
     * @return A DirectoryEntry representing a directory.
     */
    static DirectoryEntry make_directory(unsigned int id, std::string name);

    /**
     * Makes a DirectoryEntry representing a directory.
     * @param id The id.
     * @return An unnamed DirectoryEntry representing a directory.
     */
    static DirectoryEntry make_directory(unsigned int id);

    /** Constructs an empty DirectoryEntry. */
    DirectoryEntry() noexcept;

    /**
     * Constructs a DirectoryEntry.
     * @param type The Type.
     * @param id The id.
     * @param name The name.
     */
    DirectoryEntry(Type type, unsigned int id, std::string name) noexcept;

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
  };

  inline std::ostream& operator <<(
      std::ostream& out, const DirectoryEntry& entry) {
    if(entry.m_type == DirectoryEntry::Type::NONE || entry.m_id == -1) {
      return out << "NONE";
    }
    out << '(' << entry.m_type << ' ' << entry.m_id;
    if(!entry.m_name.empty()) {
      out << ' ' << entry.m_name;
    }
    return out << ')';
  }

  inline std::size_t hash_value(const DirectoryEntry& value) {
    return value.m_id;
  }

  inline const DirectoryEntry DirectoryEntry::ROOT_ACCOUNT =
    DirectoryEntry::make_account(1, "root");

  inline const DirectoryEntry DirectoryEntry::STAR_DIRECTORY =
    DirectoryEntry::make_directory(0, "*");

  inline bool DirectoryEntry::name_comparator(
      const DirectoryEntry& lhs, const DirectoryEntry& rhs) {
    return std::tie(lhs.m_name, lhs.m_id) < std::tie(rhs.m_name, rhs.m_id);
  }

  inline DirectoryEntry DirectoryEntry::make_account(
      unsigned int id, std::string name) {
    return DirectoryEntry(Type::ACCOUNT, id, std::move(name));
  }

  inline DirectoryEntry DirectoryEntry::make_account(unsigned int id) {
    return make_account(id, {});
  }

  inline DirectoryEntry DirectoryEntry::make_directory(
      unsigned int id, std::string name) {
    return DirectoryEntry(Type::DIRECTORY, id, std::move(name));
  }

  inline DirectoryEntry DirectoryEntry::make_directory(unsigned int id) {
    return make_directory(id, {});
  }

  inline DirectoryEntry::DirectoryEntry() noexcept
    : m_type(Type::NONE),
      m_id(-1) {}

  inline DirectoryEntry::DirectoryEntry(
    Type type, unsigned int id, std::string name) noexcept
    : m_type(type),
      m_id(id),
      m_name(std::move(name)) {}

  inline bool DirectoryEntry::operator <(const DirectoryEntry& rhs) const {
    return m_id < rhs.m_id;
  }

  inline bool DirectoryEntry::operator ==(const DirectoryEntry& rhs) const {
    return m_id == rhs.m_id && m_type == rhs.m_type;
  }

  template<>
  struct Shuttle<DirectoryEntry> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, DirectoryEntry& value, unsigned int version) const {
      shuttle.shuttle("type", value.m_type);
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("name", value.m_name);
    }
  };
}

namespace std {
  template <>
  struct hash<Beam::DirectoryEntry> {
    size_t operator()(const Beam::DirectoryEntry& value) const noexcept {
      return hash_value(value);
    }
  };
}

#endif
