#ifndef BEAM_SERVICE_ENTRY_HPP
#define BEAM_SERVICE_ENTRY_HPP
#include <string>
#include "Beam/Json/JsonObject.hpp"
#include "Beam/Json/JsonParser.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"

namespace Beam {

  /** Stores information about a service. */
  class ServiceEntry {
    public:

      /** Constructs an empty ServiceEntry. */
      ServiceEntry() = default;

      /**
       * Constructs a ServiceEntry.
       * @param name The name of the service.
       * @param properties The service's properties.
       * @param id The unique id assigned to the service.
       * @param account The account providing the service.
       */
      ServiceEntry(std::string name, JsonObject properties, int id,
        DirectoryEntry account) noexcept;

      /** Returns the name of the service. */
      const std::string& get_name() const;

      /** Returns the properties. */
      const JsonObject& get_properties() const;

      /** Returns the service id. */
      int get_id() const;

      /** Returns the account providing the service. */
      const DirectoryEntry& get_account() const;

      /**
       * Tests for equality.
       * @param lhs The left hand side of the test.
       * @return <code>true</code> iff <i>lhs</i> has the same id as
       *         <i>this</i>.
       */
      bool operator ==(const ServiceEntry& lhs) const;

    private:
      friend struct DataShuttle;
      std::string m_name;
      JsonObject m_properties;
      int m_id;
      DirectoryEntry m_account;

      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  inline std::ostream& operator <<(
      std::ostream& out, const ServiceEntry& entry) {
    return out << '(' << entry.get_name() << ' ' << entry.get_id() << ' ' <<
      entry.get_account() << ' ' << entry.get_properties() << ')';
  }

  inline ServiceEntry::ServiceEntry(std::string name, JsonObject properties,
    int id, DirectoryEntry account) noexcept
    : m_name(std::move(name)),
      m_properties(std::move(properties)),
      m_id(id),
      m_account(std::move(account)) {}

  inline const std::string& ServiceEntry::get_name() const {
    return m_name;
  }

  inline const JsonObject& ServiceEntry::get_properties() const {
    return m_properties;
  }

  inline int ServiceEntry::get_id() const {
    return m_id;
  }

  inline const DirectoryEntry& ServiceEntry::get_account() const {
    return m_account;
  }

  inline bool ServiceEntry::operator ==(const ServiceEntry& lhs) const {
    return m_id == lhs.m_id;
  }

  template<IsShuttle S>
  void ServiceEntry::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("properties", m_properties);
    shuttle.shuttle("id", m_id);
    shuttle.shuttle("account", m_account);
  }
}

#endif
