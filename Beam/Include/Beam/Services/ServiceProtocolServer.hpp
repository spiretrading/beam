#ifndef BEAM_SERVICE_PROTOCOL_SERVER_HPP
#define BEAM_SERVICE_PROTOCOL_SERVER_HPP
#include "Beam/Collections/SynchronizedSet.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/NativePointerPolicy.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Services/NullSession.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceSlots.hpp"
#include "Beam/TimeService/Timer.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/HashTuple.hpp"
#include "Beam/Utilities/ReportException.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * A server accepting ServiceProtocolClients.
   * @tparam C The type of ServerConnection accepting Channels.
   * @tparam S The type of Sender used for serialization.
   * @tparam E The type of Encoder used for messages.
   * @tparam T The type of Timer used for heartbeats.
   * @tparam I The type storing session info.
   * @tparam P Whether requests can be handled in parallel.
   */
  template<typename C, IsSender S, IsEncoder E, typename T,
    typename I = NullSession, bool P = false> requires
      IsServerConnection<dereference_t<C>> && IsTimer<dereference_t<T>>
  class ServiceProtocolServer {
    public:

      /** The type of ServerConnection accepting Channels. */
      using ServerConnection = dereference_t<C>;

      /** The type of Channel accepted by the ServerConnection. */
      using Channel = typename ServerConnection::Channel;

      /** The type of Sender used for serialization. */
      using Sender = S;

      /** The type of Encoder used for messages. */
      using Encoder = E;

      /** The type of MessageProtocol used to send and receive messages. */
      using MessageProtocol =
        Beam::MessageProtocol<std::unique_ptr<Channel>, Sender, Encoder>;

      /** The type of Timer used for heartbeats. */
      using Timer = dereference_t<T>;

      /** The type storing session info. */
      using Session = I;

      /** Whether requests can be handled in parallel. */
      static constexpr auto SUPPORTS_PARALLELISM = P;

      /** The type of ServiceProtocolClient accepted. */
      using ServiceProtocolClient = Beam::ServiceProtocolClient<
        MessageProtocol, T, NativePointerPolicy, Session, SUPPORTS_PARALLELISM>;

      /** Constructs Timers used for heartbeats. */
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
       * @param server_connection Initializes the ServerConnection.
       * @param timer_factory Constructs Timers for the ServiceProtocolClients.
       * @param accept_slot The slot to call when a ServiceProtocolClient is
       *        accepted.
       * @param client_closed_slot The slot to call when a ServiceProtocolClient
       *        is closed.
       */
      template<Initializes<C> CF>
      ServiceProtocolServer(CF&& server_connection, TimerFactory timer_factory,
        AcceptSlot accept_slot, ClientClosedSlot client_closed_slot);

      ~ServiceProtocolServer();

      /** Returns the ServiceSlots shared amongst all ServiceProtocolClients. */
      ServiceSlots<ServiceProtocolClient>& get_slots();

      void close();

    private:
      local_ptr_t<C> m_server_connection;
      TimerFactory m_timer_factory;
      AcceptSlot m_accept_slot;
      ClientClosedSlot m_client_closed_slot;
      ServiceSlots<ServiceProtocolClient> m_slots;
      RoutineHandler m_accept_routine;
      OpenState m_open_state;

      ServiceProtocolServer(const ServiceProtocolServer&) = delete;
      ServiceProtocolServer& operator =(const ServiceProtocolServer&) = delete;
      void accept_loop();
  };

  template<typename C, IsSender S, IsEncoder E, typename T, typename I,
    bool P> requires
      IsServerConnection<dereference_t<C>> && IsTimer<dereference_t<T>>
  template<Initializes<C> CF>
  ServiceProtocolServer<C, S, E, T, I, P>::ServiceProtocolServer(
      CF&& server_connection, TimerFactory timer_factory,
      AcceptSlot accept_slot, ClientClosedSlot client_closed_slot)
      : m_server_connection(std::forward<CF>(server_connection)),
        m_timer_factory(std::move(timer_factory)),
        m_accept_slot(std::move(accept_slot)),
        m_client_closed_slot(std::move(client_closed_slot)) {
    m_accept_routine = spawn(
      std::bind_front(&ServiceProtocolServer::accept_loop, this));
  }

  template<typename C, IsSender S, IsEncoder E, typename T, typename I,
    bool P> requires
      IsServerConnection<dereference_t<C>> && IsTimer<dereference_t<T>>
  ServiceProtocolServer<C, S, E, T, I, P>::~ServiceProtocolServer() {
    close();
  }

  template<typename C, IsSender S, IsEncoder E, typename T, typename I,
    bool P> requires
      IsServerConnection<dereference_t<C>> && IsTimer<dereference_t<T>>
  ServiceSlots<
      typename ServiceProtocolServer<C, S, E, T, I, P>::ServiceProtocolClient>&
      ServiceProtocolServer<C, S, E, T, I, P>::get_slots() {
    return m_slots;
  }

  template<typename C, IsSender S, IsEncoder E, typename T, typename I,
    bool P> requires
      IsServerConnection<dereference_t<C>> && IsTimer<dereference_t<T>>
  void ServiceProtocolServer<C, S, E, T, I, P>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_server_connection->close();
    m_accept_routine.wait();
    m_open_state.close();
  }

  template<typename C, IsSender S, IsEncoder E, typename T, typename I,
    bool P> requires
      IsServerConnection<dereference_t<C>> && IsTimer<dereference_t<T>>
  void ServiceProtocolServer<C, S, E, T, I, P>::accept_loop() {
    auto clients =
      SynchronizedUnorderedSet<std::shared_ptr<ServiceProtocolClient>>();
    auto client_routines = RoutineHandlerGroup();
    while(true) {
      auto channel = std::unique_ptr<Channel>();
      try {
        channel = m_server_connection->accept();
      } catch(const EndOfFileException&) {
        break;
      } catch(const std::exception&) {
        std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        continue;
      }
      auto client = std::make_shared<ServiceProtocolClient>(
        std::move(channel), &m_slots, m_timer_factory());
      clients.insert(client);
      client_routines.spawn([=, this, &clients] {
        try {
          m_accept_slot(*client);
          handle_messages_loop(*client);
        } catch(const std::exception&) {
          std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        }
        clients.erase(client);
        m_client_closed_slot(*client);
      });
    }
    auto pending_clients =
      std::unordered_set<std::shared_ptr<ServiceProtocolClient>>();
    clients.swap(pending_clients);
    for(auto& client : pending_clients) {
      client->close();
    }
  }
}

#endif
