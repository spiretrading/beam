#ifndef BEAM_TIMESERVICE_HPP
#define BEAM_TIMESERVICE_HPP
#include <string>

namespace Beam {
namespace TimeService {
  class FixedTimeClient;
  class IncrementalTimeClient;
  class LocalTimeClient;
  template<typename ChannelType, typename TimerType> class NtpTimeClient;
  struct TimeClient;
  class VirtualTimeClient;
  template<typename ClientType> class WrapperTimeClient;

  // Standard name for the time service.
  static const std::string SERVICE_NAME = "time_service";
}
}

#endif
