#ifndef BEAM_NTPTIMECLIENT_HPP
#define BEAM_NTPTIMECLIENT_HPP
#include <array>
#include <cstdint>
#include <vector>
#include <boost/chrono/system_clocks.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/IOException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Network/UdpSocketChannel.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/TimeService/TimeClient.hpp"
#include "Beam/TimeService/TimeService.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Threading/Timer.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {
namespace TimeService {

  //! The size of an NTP packet.
  static const std::size_t NTP_PACKET_SIZE = 48 * sizeof(std::uint8_t);

  //! The offset to the first byte of the NTP packet's origin timestamp.
  static const std::size_t ORIGIN_TIMESTAMP_OFFSET = 24;

  //! The offset to the first byte of the NTP packet's receive timestamp.
  static const std::size_t RECEIVE_TIMESTAMP_OFFSET = 32;

  //! The offset to the first byte of the NTP packet's transmit timestamp.
  static const std::size_t TRANSMIT_TIMESTAMP_OFFSET = 40;

  //! Specifies the type of array used to encode an NTP packet.
  using NtpPacket = std::array<std::uint8_t, NTP_PACKET_SIZE>;

  //! An NtpTimeClient that uses a live UDP Socket and timer.
  using LiveNtpTimeClient = NtpTimeClient<Network::UdpSocketChannel,
    Threading::LiveTimer>;

  /*! \class NtpTimeClient
      \brief Implements a TimeClient using the NTP protocol.
      \tparam ChannelType The type of Channel used to synchronize with the NTP
              server.
      \tparam TimerType The type of Timer used to specify the synchronization
              period.
   */
  template<typename ChannelType, typename TimerType>
  class NtpTimeClient : private boost::noncopyable {
    public:

      //! The type of Channel used to synchronize with the NTP server.
      using Channel = GetTryDereferenceType<ChannelType>;

      //! The type of Timer used to specify the synchronization period.
      using Timer = GetTryDereferenceType<TimerType>;

      //! Constructs an NtpTimeClient.
      /*!
        \param sources The list of NTP servers to query for time.
        \param timer Initializes the Timer used to specify the synchronization
               period.
      */
      template<typename TimerForward>
      NtpTimeClient(std::vector<std::unique_ptr<Channel>> sources,
        TimerForward&& timer);

      ~NtpTimeClient();

      boost::posix_time::ptime GetTime();

      void Open();

      void Close();

    private:
      struct Origin {
        boost::posix_time::ptime m_startTime;
        boost::chrono::steady_clock::time_point m_startPoint;
      };
      std::vector<std::unique_ptr<Channel>> m_sources;
      GetOptionalLocalPtr<TimerType> m_timer;
      Threading::Sync<Origin> m_origin;
      IO::OpenState m_openState;
      RoutineTaskQueue m_callbacks;

      void Shutdown();
      void Synchronize();
      void OnTimerExpired(Threading::Timer::Result result);
  };

  //! Converts an NTP timestamp to a POSIX timestamp.
  /*!
    \param source The first byte in the NTP timestamp.
    \return The POSIX timestamp representation of the <i>source</i>.
  */
  inline boost::posix_time::ptime PosixTimeFromNtpTime(
      const std::uint8_t* source) {
    auto iPart =
      static_cast<std::uint32_t>(source[0]) << 24 |
      static_cast<std::uint32_t>(source[1]) << 16 |
      static_cast<std::uint32_t>(source[2]) << 8 |
      static_cast<std::uint32_t>(source[3]);
    auto fPart =
      static_cast<std::uint32_t>(source[4]) << 24 |
      static_cast<std::uint32_t>(source[5]) << 16 |
      static_cast<std::uint32_t>(source[6]) << 8 |
      static_cast<std::uint32_t>(source[7]);
    if(iPart == 0 && fPart == 0) {
      return boost::posix_time::not_a_date_time;
    }
    boost::posix_time::ptime posixTime{boost::gregorian::date{1900, 1, 1},
      boost::posix_time::milliseconds(static_cast<std::uint64_t>(
      iPart * 1.0E3 + fPart * 1.0E3 / 0x100000000ULL))};
    return posixTime;
  }

  //! Converts a POSIX timestamp to an NTP timestamp.
  /*!
    \param timestamp The NTP timestamp to convert.
    \return The NTP timestamp representation.
  */
  inline std::uint64_t NtpTimeFromPosixTime(
      const boost::posix_time::ptime& timestamp) {
    auto epochTime = timestamp - boost::posix_time::ptime{
      boost::gregorian::date{1900, 1, 1}, boost::posix_time::seconds(0)};
    auto iPart = epochTime.total_seconds();
    auto fPart = static_cast<std::uint32_t>((0x100000000ULL *
      static_cast<std::uint64_t>(epochTime.fractional_seconds())) /
      boost::posix_time::time_duration::ticks_per_second());
    const std::uint32_t MASK = static_cast<unsigned char>(~0);
    std::uint64_t result = 0;
    std::uint8_t* token = reinterpret_cast<std::uint8_t*>(&result);
    token[0] = static_cast<char>((iPart & (MASK << 24)) >> 24);
    token[1] = static_cast<char>((iPart & (MASK << 16)) >> 16);
    token[2] = static_cast<char>((iPart & (MASK << 8)) >> 8);
    token[3] = static_cast<char>(iPart & MASK);
    token[4] = static_cast<char>((fPart & (MASK << 24)) >> 24);
    token[5] = static_cast<char>((fPart & (MASK << 16)) >> 16);
    token[6] = static_cast<char>((fPart & (MASK << 8)) >> 8);
    token[7] = static_cast<char>(fPart & MASK);
    return result;
  }

  //! Builds a LiveNtpTimeClient using a list of NTP server addresses.
  /*!
    \param sources The list of NTP server addresses.
    \param socketThreadPool The SocketThreadPool used by the UdpSocketChannels.
    \param timerThreadPool The TimerThreadPool used to pace the synchronization
           points.
    \return A LiveNtpTimeClient using the specified list of <i>sources</i>.
  */
  inline std::unique_ptr<LiveNtpTimeClient> MakeLiveNtpTimeClient(
      const std::vector<Network::IpAddress>& sources,
      Ref<Network::SocketThreadPool> socketThreadPool,
      Ref<Threading::TimerThreadPool> timerThreadPool) {
    std::vector<std::unique_ptr<Network::UdpSocketChannel>> channels;
    for(auto& source : sources) {
      auto channel = std::make_unique<Network::UdpSocketChannel>(source,
        Ref(socketThreadPool));
      auto settings = channel->GetSocket().GetReceiverSettings();
      settings.m_timeout = boost::posix_time::seconds(1);
      channel->GetSocket().SetReceiverSettings(settings);
      channels.push_back(std::move(channel));
    }
    return std::make_unique<LiveNtpTimeClient>(std::move(channels),
      Initialize(boost::posix_time::minutes(30), Ref(timerThreadPool)));
  }

  template<typename ChannelType, typename TimerType>
  template<typename TimerForward>
  NtpTimeClient<ChannelType, TimerType>::NtpTimeClient(
      std::vector<std::unique_ptr<Channel>> sources, TimerForward&& timer)
      : m_sources(std::move(sources)),
        m_timer{std::forward<TimerForward>(timer)} {}

  template<typename ChannelType, typename TimerType>
  NtpTimeClient<ChannelType, TimerType>::~NtpTimeClient() {
    Close();
  }

  template<typename ChannelType, typename TimerType>
  boost::posix_time::ptime NtpTimeClient<ChannelType, TimerType>::GetTime() {
    auto time = Threading::With(m_origin,
      [&] (const Origin& origin) {
        return origin.m_startTime + boost::posix_time::microseconds(
          (boost::chrono::steady_clock::now().time_since_epoch() -
          origin.m_startPoint.time_since_epoch()).count() / 1000);
      });
    return Truncate(time, boost::posix_time::milliseconds(1));
  }

  template<typename ChannelType, typename TimerType>
  void NtpTimeClient<ChannelType, TimerType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      Synchronize();
      m_timer->GetPublisher().Monitor(
        m_callbacks.GetSlot<Threading::Timer::Result>(
        std::bind(&NtpTimeClient::OnTimerExpired, this,
        std::placeholders::_1)));
      m_timer->Start();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ChannelType, typename TimerType>
  void NtpTimeClient<ChannelType, TimerType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ChannelType, typename TimerType>
  void NtpTimeClient<ChannelType, TimerType>::Shutdown() {
    m_timer->Cancel();
    m_callbacks.Break();
    m_openState.SetClosed();
  }

  template<typename ChannelType, typename TimerType>
  void NtpTimeClient<ChannelType, TimerType>::Synchronize() {
    boost::posix_time::time_duration averageOffset =
      boost::posix_time::seconds(0);
    auto count = 0;
    for(auto& source : m_sources) {
      NtpPacket requestPacket;
      requestPacket.fill(0);
      requestPacket[0] = 0x1B;
      typename Channel::Reader::Buffer readBuffer;
      try {
        source->GetConnection().Open();
      } catch(const IO::IOException&) {
        continue;
      }
      auto clientTransmissionTimestamp =
        boost::posix_time::microsec_clock::universal_time();
      auto encodedClientTransmissionTimestamp = NtpTimeFromPosixTime(
        clientTransmissionTimestamp);
      std::memcpy(requestPacket.data() + ORIGIN_TIMESTAMP_OFFSET,
        &encodedClientTransmissionTimestamp,
        sizeof(encodedClientTransmissionTimestamp));
      try {
        source->GetWriter().Write(requestPacket.data(), requestPacket.size());
        source->GetReader().Read(Store(readBuffer));
      } catch(const IO::IOException&) {
        continue;
      }
      auto clientResponseTimestamp =
        boost::posix_time::microsec_clock::universal_time();
      if(readBuffer.GetSize() != NTP_PACKET_SIZE) {
        continue;
      }
      NtpPacket responsePacket;
      std::memcpy(responsePacket.data(), readBuffer.GetData(), NTP_PACKET_SIZE);
      auto serverReceiveTimestamp = PosixTimeFromNtpTime(
        responsePacket.data() + RECEIVE_TIMESTAMP_OFFSET);
      auto serverTransmitTimestamp = PosixTimeFromNtpTime(
        responsePacket.data() + TRANSMIT_TIMESTAMP_OFFSET);
      if(serverReceiveTimestamp == boost::posix_time::not_a_date_time ||
          serverTransmitTimestamp == boost::posix_time::not_a_date_time) {
        continue;
      }
      auto roundTripDelay =
        (clientResponseTimestamp - clientTransmissionTimestamp) -
        (serverTransmitTimestamp - serverReceiveTimestamp);
      auto offset = ((serverReceiveTimestamp - clientTransmissionTimestamp) +
        (serverTransmitTimestamp - clientResponseTimestamp)) / 2;
      averageOffset += offset;
      ++count;
      source->GetConnection().Close();
    }
    if(count == 0) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Unable to query NTP time."});
    }
    averageOffset = averageOffset / count;
    Threading::With(m_origin,
      [&] (Origin& origin) {
        origin.m_startTime =
          boost::posix_time::microsec_clock::universal_time() + averageOffset;
        origin.m_startPoint = boost::chrono::steady_clock::now();
      });
  }

  template<typename ChannelType, typename TimerType>
  void NtpTimeClient<ChannelType, TimerType>::OnTimerExpired(
      Threading::Timer::Result result) {
    if(result != Threading::Timer::Result::EXPIRED) {
      return;
    }
    try {
      Synchronize();
    } catch(const std::exception&) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
    }
    m_timer->Start();
  }
}
}

#endif
