#ifndef BEAM_MESSAGE_HPP
#define BEAM_MESSAGE_HPP
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Services/ServiceSlot.hpp"

namespace Beam::Services {

  /**
   * Abstract base class for a message.
   * @param <C> The type of ServiceProtocolClient interpreting this Message.
   */
  template<typename C>
  class Message {
    public:

      /** Specifies the type of ServiceProtocolClient used. */
      using ServiceProtocolClient = C;

      virtual ~Message() = default;

      /**
       * Emits a signal for this Message.
       * @param slot The slot to call.
       * @param protocol The protocol which received the Message.
       */
      virtual void EmitSignal(BaseServiceSlot<ServiceProtocolClient>* slot,
        Ref<ServiceProtocolClient> protocol) const = 0;

    protected:

      /** Constructs an empty Message. */
      Message() = default;

    private:
      Message(const Message&) = delete;
      Message& operator =(const Message&) = delete;
  };
}

#endif
