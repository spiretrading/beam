#ifndef BEAM_SERVICES_HPP
#define BEAM_SERVICES_HPP

#define BEAM_SERVICE_PARAMETERS 10

namespace Beam::Services {
  template<typename C, typename M, typename T>
    class AuthenticatedServiceProtocolClientBuilder;
  template<typename C> class BaseServiceSlot;
  template<typename C> class HeartbeatMessage;
  template<typename C> class Message;
  template<typename C, typename S, typename E> class MessageProtocol;
  template<typename R, typename C> class RecordMessage;
  template<typename C, typename S> class RequestToken;
  template<typename R, typename P> class Service;
  template<typename C> class ServiceMessage;
  template<typename M, typename T, typename P, typename S, bool V>
    class ServiceProtocolClient;
  template<typename M, typename T> class ServiceProtocolClientBuilder;
  template<typename B> class ServiceProtocolClientHandler;
  template<typename C, typename S, typename E, typename T, typename Q, bool V>
    class ServiceProtocolServer;
  template<typename C> struct ServiceProtocolServlet;
  template<typename M, typename C, typename S, typename E, typename T,
    typename P> class ServiceProtocolServletContainer;
  class ServiceRequestException;
  template<typename M> class ServiceSlot;
  template<typename C> class ServiceSlots;
}

#endif
