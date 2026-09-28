#ifndef BEAM_SERVICE_UPDATE_HPP
#define BEAM_SERVICE_UPDATE_HPP
#include "Beam/ServiceLocator/ServiceEntry.hpp"

namespace Beam {

  /** Reports a service registration being added or removed. */
  struct ServiceUpdate {

    /** The change to the registration. */
    enum class Type {

      /** The service was registered. */
      ADDED,

      /** The service was unregistered. */
      REMOVED
    };

    /** Returns an update for a registered service. */
    static ServiceUpdate add(ServiceEntry service);

    /** Returns an update for an unregistered service. */
    static ServiceUpdate remove(ServiceEntry service);

    /** The service registration. */
    ServiceEntry m_service;

    /** The change to the registration. */
    Type m_type;

    bool operator ==(const ServiceUpdate&) const = default;
  };

  inline std::ostream& operator <<(
      std::ostream& out, ServiceUpdate::Type type) {
    if(type == ServiceUpdate::Type::ADDED) {
      return out << "ADDED";
    }
    return out << "REMOVED";
  }

  inline std::ostream& operator <<(
      std::ostream& out, const ServiceUpdate& update) {
    return out << '(' << update.m_service << ' ' << update.m_type << ')';
  }

  inline ServiceUpdate ServiceUpdate::add(ServiceEntry service) {
    return ServiceUpdate(std::move(service), Type::ADDED);
  }

  inline ServiceUpdate ServiceUpdate::remove(ServiceEntry service) {
    return ServiceUpdate(std::move(service), Type::REMOVED);
  }

  template<>
  struct Shuttle<ServiceUpdate> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, ServiceUpdate& value, unsigned int version) const {
      shuttle.shuttle("service", value.m_service);
      shuttle.shuttle("type", value.m_type);
    }
  };
}

#endif
