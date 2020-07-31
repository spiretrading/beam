#ifndef BEAM_SERVICEPROTOCOLSERVER_HPP
#define BEAM_SERVICEPROTOCOLSERVER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/Collections/SynchronizedSet.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/NativePointerPolicy.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceSlots.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/HashTuple.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {
namespace Services {

  /*! \class ServiceProtocolServer
      \brief A server accepting ServiceProtocolClients.
      \tparam ServerConnectionType The type of ServerConnection accepting
              Channels.
      \tparam SenderType The type of Sender used for serialization.
      \tparam EncoderType The type of Encoder used for messages.
      \tparam TimerType The type of Timer used for heartbeats.
      \tparam SessionType The type storing session info.
      \tparam SupportsParallelismValue Whether requests can be handled in
              parallel.

   */
  template<typename ServerConnectionType, typename SenderType,
    typename EncoderType, typename TimerType, typename SessionType = NullType,
    bool SupportsParallelismValue = false>
  class ServiceProtocolServer : private boost::noncopyable {
    public:

      //! The type of ServerConnection accepting Channels.
      using ServerConnection = GetTryDereferenceType<ServerConnectionType>;

      //! The type of Channel accepted by the ServerConnection.
      using Channel = typename ServerConnection::Channel;

      //! The type of Sender used for serialization.
      using Sender = SenderType;

      //! The type of Encoder used for messages.
      using Encoder = EncoderType;

      //! The type of MessageProtocol used to send and receive messages.
      using MessageProtocol = Services::MessageProtocol<
        std::unique_ptr<Channel>, Sender, Encoder>;

      //! The type of Timer used for heartbeats.
      using Timer = GetTryDereferenceType<TimerType>;

      //! The type storing session info.
      using Session = SessionType;

      //! Whether requests can be handled in parallel.
      static constexpr bool SupportsParallelism = SupportsParallelismValue;

      //! The type of ServiceProtocolClient accepted.
      using ServiceProtocolClient = Services::ServiceProtocolClient<
        MessageProtocol, TimerType, NativePointerPolicy, Session,
        SupportsParallelism>;

      //! Builds Timers used for heartbeats.
      using TimerFactory = std::function<TimerType ()>;

      //! Called when a ServiceProtocolClient is accepted.
      /*!
        \param client The ServiceProtocolClient that was accepted.
      */
      using AcceptSlot = std::function<void (ServiceProtocolClient& client)>;

      //! Called when a ServiceProtocolClient is closed.
      /*!
        \param client The ServiceProtocolClient that closed.
      */
      using ClientClosedSlot =
        std::function<void (ServiceProtocolClient& client)>;

      //! Constructs a ServiceProtocolServer.
      /*!
        \param serverConnection Initializes the ServerConnection.
        \param timerFactory Builds Timers for the ServiceProtocolClients.
        \param acceptSlot The slot to call when a ServiceProtocolClient is
               accepted.
        \param clientClosedSlot The slot to call when a ServiceProtocolClient is
               closed.
      */
      template<typename ServerConnectionForward>
      ServiceProtocolServer(ServerConnectionForward&& serverConnection,
        const TimerFactory& timerFactory, const AcceptSlot& acceptSlot,
        const ClientClosedSlot& clientClosedSlot);

      ~ServiceProtocolServer();

      //! Returns the ServiceSlots shared amongst all ServiceProtocolClients.
      ServiceSlots<ServiceProtocolClient>& GetSlots();

      void Open();

      void Close();

    private:
      typename OptionalLocalPtr<ServerConnectionType>::type m_serverConnection;
      TimerFactory m_timerFactory;
      AcceptSlot m_acceptSlot;
      ClientClosedSlot m_clientClosedSlot;
      ServiceSlots<ServiceProtocolClient> m_slots;
      Routines::RoutineHandler m_acceptRoutine;
      IO::OpenState m_openState;

      void Shutdown();
      void AcceptLoop();
  };

  template<typename ServerConnectionType, typename SenderType,
    typename EncoderType, typename TimerType, typename SessionType,
    bool SupportsParallelismValue>
  template<typename ServerConnectionForward>
  ServiceProtocolServer<ServerConnectionType, SenderType, EncoderType,
      TimerType, SessionType, SupportsParallelismValue>::ServiceProtocolServer(
      ServerConnectionForward&& serverConnection,
      const TimerFactory& timerFactory, const AcceptSlot& acceptSlot,
      const ClientClosedSlot& clientClosedSlot)
      : m_serverConnection(std::forward<ServerConnectionForward>(
          serverConnection)),
        m_timerFactory(timerFactory),
        m_acceptSlot(acceptSlot),
        m_clientClosedSlot(clientClosedSlot) {}

  template<typename ServerConnectionType, typename SenderType,
    typename EncoderType, typename TimerType, typename SessionType,
    bool SupportsParallelismValue>
  ServiceProtocolServer<ServerConnectionType, SenderType, EncoderType,
      TimerType, SessionType, SupportsParallelismValue>::
      ~ServiceProtocolServer() {
    Close();
  }

  template<typename ServerConnectionType, typename SenderType,
    typename EncoderType, typename TimerType, typename SessionType,
    bool SupportsParallelismValue>
  ServiceSlots<typename ServiceProtocolServer<ServerConnectionType, SenderType,
      EncoderType, TimerType, SessionType, SupportsParallelismValue>::
      ServiceProtocolClient>& ServiceProtocolServer<ServerConnectionType,
      SenderType, EncoderType, TimerType, SessionType,
      SupportsParallelismValue>::GetSlots() {
    return m_slots;
  }

  template<typename ServerConnectionType, typename SenderType,
    typename EncoderType, typename TimerType, typename SessionType,
    bool SupportsParallelismValue>
  void ServiceProtocolServer<ServerConnectionType, SenderType, EncoderType,
      TimerType, SessionType, SupportsParallelismValue>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_serverConnection->Open();
      m_acceptRoutine = Routines::Spawn(
        std::bind(&ServiceProtocolServer::AcceptLoop, this));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ServerConnectionType, typename SenderType,
    typename EncoderType, typename TimerType, typename SessionType,
    bool SupportsParallelismValue>
  void ServiceProtocolServer<ServerConnectionType, SenderType, EncoderType,
      TimerType, SessionType, SupportsParallelismValue>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ServerConnectionType, typename SenderType,
    typename EncoderType, typename TimerType, typename SessionType,
    bool SupportsParallelismValue>
  void ServiceProtocolServer<ServerConnectionType, SenderType, EncoderType,
      TimerType, SessionType, SupportsParallelismValue>::Shutdown() {
    m_serverConnection->Close();
    m_acceptRoutine.Wait();
    m_openState.SetClosed();
  }

  template<typename ServerConnectionType, typename SenderType,
    typename EncoderType, typename TimerType, typename SessionType,
    bool SupportsParallelismValue>
  void ServiceProtocolServer<ServerConnectionType, SenderType, EncoderType,
      TimerType, SessionType, SupportsParallelismValue>::AcceptLoop() {
    SynchronizedUnorderedSet<std::shared_ptr<ServiceProtocolClient>> clients;
    Routines::RoutineHandlerGroup clientRoutines;
    while(true) {
      std::unique_ptr<Channel> channel;
      try {
        channel = m_serverConnection->Accept();
      } catch(const IO::EndOfFileException&) {
        break;
      } catch(const std::exception&) {
        std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        continue;
      }
      auto client = std::make_shared<ServiceProtocolClient>(std::move(channel),
        &m_slots, m_timerFactory());
      clients.Insert(client);
      clientRoutines.Spawn(
        [=, &clients] {
          try {
            client->Open();
            m_acceptSlot(*client);
            HandleMessagesLoop(*client);
          } catch(const std::exception&) {
            std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
          }
          clients.Erase(client);
          m_clientClosedSlot(*client);
        });
    }
    std::unordered_set<std::shared_ptr<ServiceProtocolClient>> pendingClients;
    clients.Swap(pendingClients);
    for(auto& client : pendingClients) {
      client->Close();
    }
  }
}
}

#endif
