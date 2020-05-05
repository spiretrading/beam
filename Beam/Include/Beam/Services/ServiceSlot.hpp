#ifndef BEAM_SERVICESLOT_HPP
#define BEAM_SERVICESLOT_HPP
#include "Beam/Services/Services.hpp"

namespace Beam {
namespace Services {

  /*! \class BaseServiceSlot
      \brief Base class for storing a callback to a Service Message.
      \tparam ServiceProtocolClientType The type ServiceProtocolClient this slot
              is used with.
   */
  template<typename ServiceProtocolClientType>
  class BaseServiceSlot {
    public:

      //! The signature of a pre-hook.
      using PreHook = std::function<void(ServiceProtocolClientType&)>;

      virtual ~BaseServiceSlot();

      //! Adds a hook to be called prior to the service's handler.  If the hook
      //! throws an exception then the exception is sent to the client and
      //! further execution of the service is terminated.
      /*!
        \param hook The hook to call prior to the service handler.
      */
      virtual void AddPreHook(const PreHook& hook) = 0;
  };

  template<typename ServiceProtocolClientType>
  BaseServiceSlot<ServiceProtocolClientType>::~BaseServiceSlot() {}

  /*! \class ServiceSlot
      \brief Base class for a ServiceSlot for a particular Message type.
      \tparam MessageType The type of Message this slot is called for.
   */
  template<typename MessageType>
  class ServiceSlot :
      public BaseServiceSlot<typename MessageType::ServiceProtocolClient> {
    public:

      //! The type of Message this slot is called for.
      using Message = MessageType;
      using PreHook = typename BaseServiceSlot<
        typename MessageType::ServiceProtocolClient>::PreHook;

      virtual ~ServiceSlot();
  };

  template<typename MessageType>
  ServiceSlot<MessageType>::~ServiceSlot() {}
}
}

#endif
