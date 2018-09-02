#ifndef BEAM_MESSAGE_HPP
#define BEAM_MESSAGE_HPP
#include <boost/noncopyable.hpp>
#include "Beam/Services/Services.hpp"

namespace Beam {
namespace Services {

  /*! \class Message
      \brief Abstract base class for a message.
      \tparam ServiceProtocolClientType The type of ServiceProtocolClient
              interpreting this Message.
   */
  template<typename ServiceProtocolClientType>
  class Message : private boost::noncopyable {
    public:

      //! Specifies the type of ServiceProtocolClient used.
      using ServiceProtocolClient = ServiceProtocolClientType;

      virtual ~Message() = default;

      //! Emits a signal for this Message.
      /*!
        \param slot The slot to call.
        \param protocol The protocol which received the Message.
      */
      virtual void EmitSignal(BaseServiceSlot<ServiceProtocolClient>* slot,
        Ref<ServiceProtocolClient> protocol) const = 0;
  };
}
}

#endif
