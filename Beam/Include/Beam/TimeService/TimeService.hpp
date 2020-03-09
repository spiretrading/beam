#ifndef BEAM_TIME_SERVICE_HPP
#define BEAM_TIME_SERVICE_HPP
#include <string>

namespace Beam::TimeService {
  class FixedTimeClient;
  class IncrementalTimeClient;
  class LocalTimeClient;
  template<typename ChannelType, typename TimerType> class NtpTimeClient;
  struct TimeClient;
  class VirtualTimeClient;
  template<typename ClientType> class WrapperTimeClient;

  // Standard name for the time service.
  inline const std::string SERVICE_NAME = "time_service";
}

#endif
