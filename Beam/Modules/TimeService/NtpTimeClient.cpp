module;
#include "Prelude.hpp"

module Beam;

namespace Beam {
  std::unique_ptr<LiveNtpTimeClient> make_live_ntp_time_client(
      const std::vector<IpAddress>& sources,
      boost::posix_time::time_duration sync_period) {
    auto channels = try_or_nest([&] {
      auto channels = std::vector<std::unique_ptr<UdpSocketChannel>>();
      auto options = UdpSocketOptions();
      options.m_timeout = boost::posix_time::seconds(1);
      for(auto& source : sources) {
        auto channel = std::make_unique<UdpSocketChannel>(source, options);
        channels.push_back(std::move(channel));
      }
      return channels;
    }, ConnectException("Unable to connect to NTP service."));
    return std::make_unique<LiveNtpTimeClient>(
      std::move(channels), init(sync_period));
  }
}
