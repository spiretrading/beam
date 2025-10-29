#ifndef BEAM_NTP_TIME_CLIENT_HPP
#define BEAM_NTP_TIME_CLIENT_HPP
#include <array>
#include <cstdint>
#include <vector>
#include <boost/chrono/system_clocks.hpp>
#include <boost/endian.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/IOException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/UdpSocketChannel.hpp"
#include "Beam/Parsers/Parse.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/TimeService/LiveTimer.hpp"
#include "Beam/TimeService/TimeClient.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/ReportException.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** The size of an NTP packet. */
  static constexpr auto NTP_PACKET_SIZE = 48 * sizeof(std::uint8_t);

  /** The offset to the first byte of the NTP packet's origin timestamp. */
  static constexpr auto ORIGIN_TIMESTAMP_OFFSET = std::size_t(24);

  /** The offset to the first byte of the NTP packet's receive timestamp. */
  static constexpr auto RECEIVE_TIMESTAMP_OFFSET = std::size_t(32);

  /** The offset to the first byte of the NTP packet's transmit timestamp. */
  static constexpr auto TRANSMIT_TIMESTAMP_OFFSET = std::size_t(40);

  /** Specifies the type of array used to encode an NTP packet. */
  using NtpPacket = std::array<std::uint8_t, NTP_PACKET_SIZE>;

  /**
   * Implements a TimeClient using the NTP protocol.
   * @param C The type of Channel used to synchronize with the NTP server.
   * @param T The type of Timer used to specify the synchronization period.
   */
  template<typename C, typename T> requires
    IsChannel<dereference_t<C>> && IsTimer<dereference_t<T>>
  class NtpTimeClient {
    public:

      /** The type of Channel used to synchronize with the NTP server. */
      using Channel = dereference_t<C>;

      /** The type of Timer used to specify the synchronization period. */
      using Timer = dereference_t<T>;

      /**
       * Constructs an NtpTimeClient.
       * @param sources The list of NTP servers to query for time.
       * @param timer Initializes the Timer used to specify the synchronization
       *        period.
       */
      template<Initializes<T> TF>
      NtpTimeClient(std::vector<std::unique_ptr<Channel>> sources, TF&& timer);

      ~NtpTimeClient();

      boost::posix_time::ptime get_time();
      void close();

    private:
      struct Origin {
        boost::posix_time::ptime m_start_time;
        boost::chrono::steady_clock::time_point m_start;
      };
      std::vector<std::unique_ptr<Channel>> m_sources;
      local_ptr_t<T> m_timer;
      Sync<Origin> m_origin;
      OpenState m_open_state;
      RoutineTaskQueue m_tasks;

      NtpTimeClient(const NtpTimeClient&) = delete;
      NtpTimeClient& operator =(const NtpTimeClient&) = delete;
      void synchronize();
      void on_timer_expired(Beam::Timer::Result result);
  };

  /** An NtpTimeClient that uses a live UDP Socket and timer. */
  using LiveNtpTimeClient = NtpTimeClient<UdpSocketChannel, LiveTimer>;

  /**
   * Converts an NTP timestamp to a POSIX timestamp.
   * @param source The first byte in the NTP timestamp.
   * @return The POSIX timestamp representation of the <i>source</i>.
   */
  inline boost::posix_time::ptime posix_time_from_ntp_time(
      const std::uint8_t* source) {
    auto integer_part = std::uint32_t(0);
    auto fractional_part = std::uint32_t(0);
    std::memcpy(&integer_part, source, sizeof(integer_part));
    std::memcpy(&fractional_part, source + 4, sizeof(fractional_part));
    boost::endian::big_to_native_inplace(integer_part);
    boost::endian::big_to_native_inplace(fractional_part);
    if(integer_part == 0 && fractional_part == 0) {
      return boost::posix_time::not_a_date_time;
    }
    auto timestamp = boost::posix_time::ptime(
      boost::gregorian::date(1900, 1, 1),
      boost::posix_time::milliseconds(static_cast<std::uint64_t>(
        integer_part * 1.0E3 + fractional_part * 1.0E3 / 0x100000000ULL)));
    return timestamp;
  }

  /**
   * Converts a POSIX timestamp to an NTP timestamp.
   * @param timestamp The NTP timestamp to convert.
   * @return The NTP timestamp representation.
   */
  inline std::uint64_t ntp_time_from_posix_time(
      boost::posix_time::ptime timestamp) {
    auto epoch_time = timestamp - boost::posix_time::ptime(
      boost::gregorian::date(1900, 1, 1), boost::posix_time::seconds(0));
    auto integer_part = static_cast<std::uint32_t>(epoch_time.total_seconds());
    auto fractional_part = static_cast<std::uint32_t>((0x100000000ULL *
      static_cast<std::uint64_t>(epoch_time.fractional_seconds())) /
        boost::posix_time::time_duration::ticks_per_second());
    auto big_endian_integer = boost::endian::native_to_big(integer_part);
    auto big_endian_fractional = boost::endian::native_to_big(fractional_part);
    auto result = std::uint64_t(0);
    std::memcpy(&result, &big_endian_integer, sizeof(big_endian_integer));
    std::memcpy(
      reinterpret_cast<std::uint8_t*>(&result) + sizeof(big_endian_integer),
      &big_endian_fractional, sizeof(big_endian_fractional));
    return result;
  }

  /**
   * Returns a LiveNtpTimeClient using a list of NTP server addresses.
   * @param sources The list of NTP server addresses.
   * @param sync_period The amount of time to wait before synchronizing the
   *        time.
   * @return A LiveNtpTimeClient using the specified list of <i>sources</i>.
   */
  inline std::unique_ptr<LiveNtpTimeClient> make_live_ntp_time_client(
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

  /**
   * Returns a LiveNtpTimeClient using a list of NTP server addresses.
   * @param sources The list of NTP server addresses.
   * @return A LiveNtpTimeClient using the specified list of <i>sources</i>.
   */
  inline std::unique_ptr<LiveNtpTimeClient> make_live_ntp_time_client(
      const std::vector<IpAddress>& sources) {
    return make_live_ntp_time_client(sources, boost::posix_time::minutes(30));
  }

  /**
   * Returns a LiveNtpTimeClient by checking the ServiceLocatorClient for a list
   * of NTP servers.
   * @param client The ServiceLocatorClient used to locate NTP servers.
   * @return A LiveNtpTimeClient using the specified list of <i>sources</i>.
   */
  template<IsServiceLocatorClient T>
  std::unique_ptr<LiveNtpTimeClient> make_live_ntp_time_client(T& client) {
    auto ntp_pool = try_or_nest([&] {
      auto services = client.locate(TIME_SERVICE_NAME);
      if(services.empty()) {
        boost::throw_with_location(
          std::runtime_error("No time services available."));
      }
      auto& service = services.front();
      return parse<std::vector<IpAddress>>(
        boost::get<std::string>(service.get_properties().at("addresses")));
    }, ConnectException("Unable to connect to NTP service."));
    return make_live_ntp_time_client(ntp_pool);
  }

  template<typename C, typename T> requires
    IsChannel<dereference_t<C>> && IsTimer<dereference_t<T>>
  template<Initializes<T> TF>
  NtpTimeClient<C, T>::NtpTimeClient(
      std::vector<std::unique_ptr<Channel>> sources, TF&& timer)
      try : m_sources(std::move(sources)),
            m_timer(std::forward<TF>(timer)) {
    try {
      synchronize();
      m_timer->get_publisher().monitor(m_tasks.get_slot<Beam::Timer::Result>(
        std::bind_front(&NtpTimeClient::on_timer_expired, this)));
      m_timer->start();
    } catch(const std::exception&) {
      close();
      throw;
    }
  } catch(const std::exception&) {
    std::throw_with_nested(ConnectException("Unable to open NTP service."));
  }

  template<typename C, typename T> requires
    IsChannel<dereference_t<C>> && IsTimer<dereference_t<T>>
  NtpTimeClient<C, T>::~NtpTimeClient() {
    close();
  }

  template<typename C, typename T> requires
    IsChannel<dereference_t<C>> && IsTimer<dereference_t<T>>
  boost::posix_time::ptime NtpTimeClient<C, T>::get_time() {
    m_open_state.ensure_open();
    auto time = with(m_origin, [&] (const auto& origin) {
      return origin.m_start_time + boost::posix_time::microseconds(
        (boost::chrono::steady_clock::now().time_since_epoch() -
        origin.m_start.time_since_epoch()).count() / 1000);
    });
    return truncate(time, boost::posix_time::milliseconds(1));
  }

  template<typename C, typename T> requires
    IsChannel<dereference_t<C>> && IsTimer<dereference_t<T>>
  void NtpTimeClient<C, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_timer->cancel();
    m_tasks.close();
    m_tasks.wait();
    m_open_state.close();
  }

  template<typename C, typename T> requires
    IsChannel<dereference_t<C>> && IsTimer<dereference_t<T>>
  void NtpTimeClient<C, T>::synchronize() {
    auto average_offset =
      boost::posix_time::time_duration(boost::posix_time::seconds(0));
    auto count = 0;
    for(auto& source : m_sources) {
      auto request_packet = NtpPacket();
      request_packet.fill(0);
      request_packet[0] = 0x1B;
      auto read_buffer = SharedBuffer();
      auto client_transmission_timestamp =
        boost::posix_time::microsec_clock::universal_time();
      auto encoded_client_transmission_timestamp =
        ntp_time_from_posix_time(client_transmission_timestamp);
      std::memcpy(request_packet.data() + TRANSMIT_TIMESTAMP_OFFSET,
        &encoded_client_transmission_timestamp,
        sizeof(encoded_client_transmission_timestamp));
      try {
        write(source->get_writer(), request_packet);
        source->get_reader().read(out(read_buffer));
      } catch(const IOException&) {
        continue;
      }
      auto client_response_timestamp =
        boost::posix_time::microsec_clock::universal_time();
      if(read_buffer.get_size() != NTP_PACKET_SIZE) {
        continue;
      }
      auto response_packet = NtpPacket();
      std::memcpy(
        response_packet.data(), read_buffer.get_data(), NTP_PACKET_SIZE);
      auto server_receive_timestamp = posix_time_from_ntp_time(
        response_packet.data() + RECEIVE_TIMESTAMP_OFFSET);
      auto server_transmit_timestamp = posix_time_from_ntp_time(
        response_packet.data() + TRANSMIT_TIMESTAMP_OFFSET);
      if(server_receive_timestamp == boost::posix_time::not_a_date_time ||
          server_transmit_timestamp == boost::posix_time::not_a_date_time) {
        continue;
      }
      auto round_trip_delay =
        (client_response_timestamp - client_transmission_timestamp) -
        (server_transmit_timestamp - server_receive_timestamp);
      if(round_trip_delay.total_milliseconds() >= 1000 ||
          round_trip_delay.is_negative()) {
        continue;
      }
      auto offset =
        ((server_receive_timestamp - client_transmission_timestamp) +
        (server_transmit_timestamp - client_response_timestamp)) / 2;
      average_offset += offset;
      ++count;
    }
    if(count == 0) {
      boost::throw_with_location(IOException("Unable to query NTP time."));
    }
    average_offset = average_offset / count;
    with(m_origin, [&] (auto& origin) {
      origin.m_start_time =
        boost::posix_time::microsec_clock::universal_time() + average_offset;
      origin.m_start = boost::chrono::steady_clock::now();
    });
  }

  template<typename C, typename T> requires
    IsChannel<dereference_t<C>> && IsTimer<dereference_t<T>>
  void NtpTimeClient<C, T>::on_timer_expired(Beam::Timer::Result result) {
    if(result != Beam::Timer::Result::EXPIRED || !m_open_state.is_open()) {
      return;
    }
    try {
      synchronize();
    } catch(const std::exception&) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
    }
    m_timer->start();
  }
}

#endif
