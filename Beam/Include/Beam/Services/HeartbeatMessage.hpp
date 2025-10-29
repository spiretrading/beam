#ifndef BEAM_HEARTBEAT_MESSAGE_HPP
#define BEAM_HEARTBEAT_MESSAGE_HPP
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Services/Message.hpp"

namespace Beam {

  /**
   * Represents a heartbeat.
   * @tparam C The type of ServiceProtocolClient interpreting this Message.
   */
  template<typename C>
  class HeartbeatMessage : public Message<C> {
    public:

      /** Specifies the type of ServiceProtocolClient used. */
      using ServiceProtocolClient = typename Message<C>::ServiceProtocolClient;

      /** Constructs a HeartbeatMessage. */
      HeartbeatMessage() = default;

      void emit(BaseServiceSlot<ServiceProtocolClient>* slot,
        Ref<ServiceProtocolClient> client) const override;

    private:
      friend struct DataShuttle;

      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<typename C>
  void HeartbeatMessage<C>::emit(BaseServiceSlot<ServiceProtocolClient>* slot,
    Ref<ServiceProtocolClient> client) const {}

  template<typename C>
  template<IsShuttle S>
  void HeartbeatMessage<C>::shuttle(S& shuttle, unsigned int version) {}
}

#endif
