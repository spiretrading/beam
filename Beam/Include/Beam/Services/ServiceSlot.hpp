#ifndef BEAM_SERVICE_SLOT_HPP
#define BEAM_SERVICE_SLOT_HPP
#include "Beam/Services/Services.hpp"

namespace Beam::Services {

  /**
   * Base class for storing a callback to a Service Message.
   * @param <C> The type ServiceProtocolClient this slot is used with.
   */
  template<typename C>
  class BaseServiceSlot {
    public:

      /** The signature of a pre-hook. */
      using PreHook = std::function<void(C&)>;

      virtual ~BaseServiceSlot() = default;

      /**
       * Adds a hook to be called prior to the service's handler. If the hook
       * throws an exception then the exception is sent to the client and
       * further execution of the service is terminated.
       * @param hook The hook to call prior to the service handler.
       */
      virtual void AddPreHook(const PreHook& hook) = 0;
  };

  /**
   * Base class for a ServiceSlot for a particular Message type.
   * @param <M> The type of Message this slot is called for.
   */
  template<typename M>
  class ServiceSlot :
      public BaseServiceSlot<typename M::ServiceProtocolClient> {
    public:

      /** The type of Message this slot is called for. */
      using Message = M;
      using PreHook = typename BaseServiceSlot<
        typename Message::ServiceProtocolClient>::PreHook;
  };
}

#endif
