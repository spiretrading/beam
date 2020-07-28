#ifndef BEAM_SERVICE_ENTRY_HPP
#define BEAM_SERVICE_ENTRY_HPP
#include <string>
#include "Beam/Json/JsonObject.hpp"
#include "Beam/Json/JsonParser.hpp"
#include "Beam/Serialization/Serialization.hpp"
#include "Beam/Serialization/ShuttlePropertyTree.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {

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
        DirectoryEntry account);

      /** Returns the name of the service. */
      const std::string& GetName() const;

      /** Returns the properties. */
      const JsonObject& GetProperties() const;

      /** Returns the service id. */
      int GetId() const;

      /** Returns the account providing the service. */
      const DirectoryEntry& GetAccount() const;

      /**
       * Tests for equality.
       * @param lhs The left hand side of the test.
       * @return <code>true</code> iff <i>lhs</i> has the same id as
       *         <i>this</i>.
       */
      bool operator ==(const ServiceEntry& lhs) const;

      /**
       * Tests for inequality.
       * @param lhs The left hand side of the test.
       * @return <code>true</code> iff <i>lhs</i> does not have the same id as
       *         <i>this</i>.
       */
      bool operator !=(const ServiceEntry& lhs) const;

    private:
      friend struct Serialization::DataShuttle;
      std::string m_name;
      JsonObject m_properties;
      int m_id;
      DirectoryEntry m_account;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline ServiceEntry::ServiceEntry(std::string name, JsonObject properties,
    int id, DirectoryEntry account)
    : m_name(std::move(name)),
      m_properties(std::move(properties)),
      m_id(id),
      m_account(std::move(account)) {}

  inline const std::string& ServiceEntry::GetName() const {
    return m_name;
  }

  inline const JsonObject& ServiceEntry::GetProperties() const {
    return m_properties;
  }

  inline int ServiceEntry::GetId() const {
    return m_id;
  }

  inline const DirectoryEntry& ServiceEntry::GetAccount() const {
    return m_account;
  }

  inline bool ServiceEntry::operator ==(const ServiceEntry& lhs) const {
    return m_id == lhs.m_id;
  }

  inline bool ServiceEntry::operator !=(const ServiceEntry& lhs) const {
    return !(*this == lhs);
  }

  template<typename Shuttler>
  void ServiceEntry::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("properties", m_properties);
    shuttle.Shuttle("id", m_id);
    shuttle.Shuttle("account", m_account);
  }
}

#endif
