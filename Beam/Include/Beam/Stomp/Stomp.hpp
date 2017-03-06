#ifndef BEAM_STOMP_HPP
#define BEAM_STOMP_HPP

namespace Beam {
namespace Stomp {
  enum class StompCommand;
  class StompException;
  class StompFrame;
  class StompFrameParser;
  class StompHeader;
  template<typename ChannelType> class StompServer;
}
}

#endif
