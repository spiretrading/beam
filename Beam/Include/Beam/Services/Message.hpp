#ifndef BEAM_MESSAGE_HPP
#define BEAM_MESSAGE_HPP
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Services/ServiceSlot.hpp"

namespace Beam {

  /**
   * Abstract base class for a message.
   * @tparam C The type of ServiceProtocolClient interpreting this Message.
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
       * @param client The client which received the Message.
       */
      virtual void emit(BaseServiceSlot<ServiceProtocolClient>* slot,
        Ref<ServiceProtocolClient> client) const = 0;

    protected:

      /** Constructs an empty Message. */
      Message() = default;

    private:
      Message(const Message&) = delete;
      Message& operator =(const Message&) = delete;
  };
}

#endif
