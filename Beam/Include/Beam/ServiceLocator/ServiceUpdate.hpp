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

template<>
class std::formatter<Beam::ServiceUpdate::Type> :
    public std::formatter<std::string_view> {
  public:
    auto format(Beam::ServiceUpdate::Type value, auto& context) const {
      auto name = [&] {
        if(value == Beam::ServiceUpdate::Type::ADDED) {
          return std::string_view("ADDED");
        }
        return std::string_view("REMOVED");
      }();
      return std::formatter<std::string_view>::format(name, context);
    }
};

template<>
class std::formatter<Beam::ServiceUpdate> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Beam::ServiceUpdate& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format("({} {})", value.m_service, value.m_type), context);
    }
};

namespace Beam {
  inline std::ostream& operator <<(
      std::ostream& out, ServiceUpdate::Type type) {
    return out << std::format("{}", type);
  }

  inline std::ostream& operator <<(
      std::ostream& out, const ServiceUpdate& update) {
    return out << std::format("{}", update);
  }
}

#endif
