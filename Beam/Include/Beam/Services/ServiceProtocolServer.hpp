#ifndef BEAM_SERVICE_PROTOCOL_SERVER_HPP
#define BEAM_SERVICE_PROTOCOL_SERVER_HPP
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

namespace Beam::Services {

  /**
   * A server accepting ServiceProtocolClients.
   * @param <C> The type of ServerConnection accepting Channels.
   * @param <S> The type of Sender used for serialization.
   * @param <E> The type of Encoder used for messages.
   * @param <T> The type of Timer used for heartbeats.
   * @param <I> The type storing session info.
   * @param <P> Whether requests can be handled in parallel.
   */
  template<typename C, typename S, typename E, typename T,
    typename I = NullType, bool P = false>
  class ServiceProtocolServer {
    public:

      /** The type of ServerConnection accepting Channels. */
      using ServerConnection = GetTryDereferenceType<C>;

      /** The type of Channel accepted by the ServerConnection. */
      using Channel = typename ServerConnection::Channel;

      /** The type of Sender used for serialization. */
      using Sender = S;

      /** The type of Encoder used for messages. */
      using Encoder = E;

      /** The type of MessageProtocol used to send and receive messages. */
      using MessageProtocol = Services::MessageProtocol<
        std::unique_ptr<Channel>, Sender, Encoder>;

      /** The type of Timer used for heartbeats. */
      using Timer = GetTryDereferenceType<T>;

      /** The type storing session info. */
      using Session = I;

      /** Whether requests can be handled in parallel. */
      static constexpr auto SupportsParallelism = P;

      /** The type of ServiceProtocolClient accepted. */
      using ServiceProtocolClient = Services::ServiceProtocolClient<
        MessageProtocol, T, NativePointerPolicy, Session, SupportsParallelism>;

      /** Builds Timers used for heartbeats. */
      using TimerFactory = std::function<T ()>;

      /**
       * Called when a ServiceProtocolClient is accepted.
       * @param client The ServiceProtocolClient that was accepted.
       */
      using AcceptSlot = std::function<void (ServiceProtocolClient& client)>;

      /**
       * Called when a ServiceProtocolClient is closed.
       * @param client The ServiceProtocolClient that closed.
       */
      using ClientClosedSlot =
        std::function<void (ServiceProtocolClient& client)>;

      /**
       * Constructs a ServiceProtocolServer.
       * @param serverConnection Initializes the ServerConnection.
       * @param timerFactory Builds Timers for the ServiceProtocolClients.
       * @param acceptSlot The slot to call when a ServiceProtocolClient is
       *        accepted.
       * @param clientClosedSlot The slot to call when a ServiceProtocolClient
       *        is closed.
       */
      template<typename CF>
      ServiceProtocolServer(CF&& serverConnection, TimerFactory timerFactory,
        AcceptSlot acceptSlot, ClientClosedSlot clientClosedSlot);

      ~ServiceProtocolServer();

      /** Returns the ServiceSlots shared amongst all ServiceProtocolClients. */
      ServiceSlots<ServiceProtocolClient>& GetSlots();

      void Close();

    private:
      typename OptionalLocalPtr<C>::type m_serverConnection;
      TimerFactory m_timerFactory;
      AcceptSlot m_acceptSlot;
      ClientClosedSlot m_clientClosedSlot;
      ServiceSlots<ServiceProtocolClient> m_slots;
      Routines::RoutineHandler m_acceptRoutine;
      IO::OpenState m_openState;

      ServiceProtocolServer(const ServiceProtocolServer&) = delete;
      ServiceProtocolServer& operator =(const ServiceProtocolServer&) = delete;
      void AcceptLoop();
  };

  template<typename C, typename S, typename E, typename T, typename I, bool P>
  template<typename CF>
  ServiceProtocolServer<C, S, E, T, I, P>::ServiceProtocolServer(
    CF&& serverConnection, TimerFactory timerFactory, AcceptSlot acceptSlot,
    ClientClosedSlot clientClosedSlot)
    : m_serverConnection(std::forward<CF>(serverConnection)),
      m_timerFactory(std::move(timerFactory)),
      m_acceptSlot(std::move(acceptSlot)),
      m_clientClosedSlot(std::move(clientClosedSlot)),
      m_acceptRoutine(Routines::Spawn(
        std::bind(&ServiceProtocolServer::AcceptLoop, this))) {}

  template<typename C, typename S, typename E, typename T, typename I, bool P>
  ServiceProtocolServer<C, S, E, T, I, P>::~ServiceProtocolServer() {
    Close();
  }

  template<typename C, typename S, typename E, typename T, typename I, bool P>
  ServiceSlots<
      typename ServiceProtocolServer<C, S, E, T, I, P>::ServiceProtocolClient>&
      ServiceProtocolServer<C, S, E, T, I, P>::GetSlots() {
    return m_slots;
  }

  template<typename C, typename S, typename E, typename T, typename I, bool P>
  void ServiceProtocolServer<C, S, E, T, I, P>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_serverConnection->Close();
    m_acceptRoutine.Wait();
    m_openState.Close();
  }

  template<typename C, typename S, typename E, typename T, typename I, bool P>
  void ServiceProtocolServer<C, S, E, T, I, P>::AcceptLoop() {
    auto clients = SynchronizedUnorderedSet<
      std::shared_ptr<ServiceProtocolClient>>();
    auto clientRoutines = Routines::RoutineHandlerGroup();
    while(true) {
      auto channel = std::unique_ptr<Channel>();
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
      clientRoutines.Spawn([=, &clients] {
        try {
          m_acceptSlot(*client);
          HandleMessagesLoop(*client);
        } catch(const std::exception&) {
          std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        }
        clients.Erase(client);
        m_clientClosedSlot(*client);
      });
    }
    auto pendingClients = std::unordered_set<
      std::shared_ptr<ServiceProtocolClient>>();
    clients.Swap(pendingClients);
    for(auto& client : pendingClients) {
      client->Close();
    }
  }
}

#endif
