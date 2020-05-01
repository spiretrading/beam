#ifndef BEAM_HEARTBEATMESSAGE_HPP
#define BEAM_HEARTBEATMESSAGE_HPP
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Services/Message.hpp"
#include "Beam/Services/Services.hpp"

namespace Beam {
namespace Services {

  /*! \class HeartbeatMessage
      \brief Represents a heartbeat.
      \tparam ServiceProtocolClientType The type of ServiceProtocolClient
              interpreting this Message.
   */
  template<typename ServiceProtocolClientType>
  class HeartbeatMessage : public Message<ServiceProtocolClientType> {
    public:

      //! Specifies the type of ServiceProtocolClient used.
      using ServiceProtocolClient = ServiceProtocolClientType;

      //! Constructs a HeartbeatMessage.
      HeartbeatMessage();

      virtual void EmitSignal(BaseServiceSlot<ServiceProtocolClient>* slot,
        Ref<ServiceProtocolClient> protocol) const;

    private:
      friend struct Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename ServiceProtocolClientType>
  HeartbeatMessage<ServiceProtocolClientType>::HeartbeatMessage() {}

  template<typename ServiceProtocolClientType>
  void HeartbeatMessage<ServiceProtocolClientType>::EmitSignal(
    BaseServiceSlot<ServiceProtocolClient>* slot,
    Ref<ServiceProtocolClient> protocol) const {}

  template<typename ServiceProtocolClientType>
  template<typename Shuttler>
  void HeartbeatMessage<ServiceProtocolClientType>::Shuttle(Shuttler& shuttle,
      unsigned int version) {}
}
}

#endif
