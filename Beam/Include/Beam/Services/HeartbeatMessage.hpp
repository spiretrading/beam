#ifndef BEAM_HEARTBEAT_MESSAGE_HPP
#define BEAM_HEARTBEAT_MESSAGE_HPP
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Services/Message.hpp"
#include "Beam/Services/Services.hpp"

namespace Beam::Services {

  /**
   * Represents a heartbeat.
   * @param <C> The type of ServiceProtocolClient interpreting this Message.
   */
  template<typename C>
  class HeartbeatMessage : public Message<C> {
    public:

      /** Specifies the type of ServiceProtocolClient used. */
      using ServiceProtocolClient = typename Message<C>::ServiceProtocolClient;

      /** Constructs a HeartbeatMessage. */
      HeartbeatMessage() = default;

      void EmitSignal(BaseServiceSlot<ServiceProtocolClient>* slot,
        Ref<ServiceProtocolClient> protocol) const override;

    private:
      friend struct Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename C>
  void HeartbeatMessage<C>::EmitSignal(
    BaseServiceSlot<ServiceProtocolClient>* slot,
    Ref<ServiceProtocolClient> protocol) const {}

  template<typename C>
  template<typename Shuttler>
  void HeartbeatMessage<C>::Shuttle(Shuttler& shuttle, unsigned int version) {}
}

#endif
