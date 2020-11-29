#ifndef BEAM_TIME_SERVICE_HPP
#define BEAM_TIME_SERVICE_HPP
#include <string>

namespace Beam::TimeService {
  class FixedTimeClient;
  class IncrementalTimeClient;
  class LocalTimeClient;
  template<typename ChannelType, typename TimerType> class NtpTimeClient;
  struct TimeClient;
  class TimeClientBox;

  /** Standard name for the time service. */
  inline const auto SERVICE_NAME = std::string("time_service");
}

#endif
