#ifndef BEAM_SERVICESTESTS_HPP
#define BEAM_SERVICESTESTS_HPP
#include "Beam/Services/Services.hpp"

namespace Beam {
namespace Services {
namespace Tests {
  class DatagramMessageProtocolTester;
  class ServiceProtocolClientTester;
  class ServiceProtocolServerTester;
  class ServiceProtocolServletContainerTester;
  template<typename ChannelType> class TestServlet;
}
}
}

#endif
