#ifndef BEAM_SERVICES_HPP
#define BEAM_SERVICES_HPP

#define BEAM_SERVICE_PARAMETERS 10

namespace Beam::Services {
  template<typename ServiceLocatorClientType, typename MessageProtocolType,
    typename TimerType> class AuthenticatedServiceProtocolClientBuilder;
  template<typename ServiceProtocolClientType> class BaseServiceSlot;
  template<typename ServiceProtocolClientType> class HeartbeatMessage;
  template<typename ServiceProtocolClientType> class Message;
  template<typename ChannelType, typename SenderType, typename EncoderType>
    class MessageProtocol;
  template<typename RecordType, typename ServiceProtocolClientType>
    class RecordMessage;
  template<typename ServiceProtocolClientType, typename ServiceType>
    class RequestToken;
  template<typename ReturnType, typename ParametersType> class Service;
  template<typename ServiceProtocolClientType> class ServiceMessage;
  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue> class ServiceProtocolClient;
  template<typename MessageProtocolType, typename TimerType>
    class ServiceProtocolClientBuilder;
  template<typename ServiceProtocolClientBuilderType>
    class ServiceProtocolClientHandler;
  template<typename ServerConnectionType, typename SenderType,
    typename EncoderType, typename TimerType, typename SessionType,
    bool SupportsParallelismValue> class ServiceProtocolServer;
  template<typename ChannelType> struct ServiceProtocolServlet;
  template<typename M, typename C, typename S, typename E, typename T,
    typename P> class ServiceProtocolServletContainer;
  class ServiceRequestException;
  template<typename MessageType> class ServiceSlot;
  template<typename ServiceProtocolClientType> class ServiceSlots;
}

#endif
