#ifndef BEAM_SERVICE_PROTOCOL_CLIENT_HPP
#define BEAM_SERVICE_PROTOCOL_CLIENT_HPP
#include <atomic>
#include <iostream>
#include <unordered_map>
#include <boost/range/adaptor/map.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Pointers/LocalPointerPolicy.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Serialization/ShuttleClone.hpp"
#include "Beam/Serialization/ShuttleUniquePtr.hpp"
#include "Beam/Serialization/TypeNotFoundException.hpp"
#include "Beam/Services/HeartbeatMessage.hpp"
#include "Beam/Services/Message.hpp"
#include "Beam/Services/MessageProtocol.hpp"
#include "Beam/Services/RecordMessage.hpp"
#include "Beam/Services/Service.hpp"
#include "Beam/Services/ServiceRequestException.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Services/ServiceSlots.hpp"
#include "Beam/Threading/Timer.hpp"
#include "Beam/Utilities/NullType.hpp"
#include "Beam/Utilities/ReportException.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"

namespace Beam::Services {
namespace Details {
  BEAM_DEFINE_HAS_VARIABLE(HasParallelism, SupportsParallelism);
}

  /**
   * Implements the service protocol on top of a Channel.
   * @param M The type of MessageProtocol used to send and receive messages.
   * @param T The type of Timer used for heartbeats.
   * @param P The pointer policy used for ServiceSlots.
   * @param S Stores session information.
   * @param V Whether this client supports handling messages in parallel.
   */
  template<typename M, typename T, typename P = LocalPointerPolicy,
    typename S = NullType, bool V = false>
  class ServiceProtocolClient {
    public:

      /** The type of MessageProtocol used to send and receive messages. */
      using MessageProtocol = M;

      /** The type of ServiceSlots used. */
      using ServiceSlots = Services::ServiceSlots<ServiceProtocolClient>;

      /** The type of Timer used. */
      using Timer = T;

      /** The type of Session info stored. */
      using Session = S;

      /** Whether this client supports handling messages in parallel. */
      static constexpr auto SupportsParallelism = V;

      /**
       * Constructs a ServiceProtocolClient.
       * @param channel Initializes the client's Channel.
       * @param slots Initializes the ServiceSlots.
       * @param timer The type of Timer used.
       */
      template<typename CF, typename SF, typename TF>
      ServiceProtocolClient(CF&& channel, SF&& slots, TF&& timer);

      /**
       * Constructs a ServiceProtocolClient.
       * @param channel Initializes the client's Channel.
       * @param timer The type of Timer used.
       */
      template<typename CF, typename TF>
      ServiceProtocolClient(CF&& channel, TF&& timer);

      ~ServiceProtocolClient();

      /** Returns the ServiceSlots. */
      const ServiceSlots& GetSlots() const;

      /** Returns the ServiceSlots. */
      ServiceSlots& GetSlots();

      /** Returns the session info. */
      const Session& GetSession() const;

      /** Returns the session info. */
      Session& GetSession();

      /**
       * Clones a ServiceRequestException usable with this protocol.
       * @param e The ServiceRequestException to clone.
       */
      std::unique_ptr<ServiceRequestException> CloneException(
        const ServiceRequestException& e);

      /**
       * Encodes a Message using this client's MessageProtocol.
       * @param message The Message to encode.
       * @param buffer The Buffer to store the encoded Message in.
       */
      template<typename Buffer>
      void Encode(const Message<ServiceProtocolClient>& message,
        Out<Buffer> buffer);

      /**
       * Sends a Message.
       * @param message The Message to send.
       */
      void Send(const Message<ServiceProtocolClient>& message);

      /**
       * Sends a Buffer.
       * @param buffer The Buffer to send.
       */
      template<typename Buffer, typename =
        std::enable_if_t<ImplementsConcept<Buffer, IO::Buffer>::value>>
      void Send(const Buffer& buffer);

      /**
       * Sends a request for a Service.
       * @param parameters The Service's parameters.
       * @return The response to this Service::Request.
       */
      template<typename Service>
      GetStorageType<typename Service::Return> SendServiceRequest(
        const typename Service::Parameters& parameters);

      /**
       * Sends a request for a Service.
       * @param args The parameters to send.
       * @return The response to this Service::Request.
       */
      template<typename Service, typename... Args>
      GetStorageType<typename Service::Return> SendRequest(Args&&... args);

      /** Reads a Message from the Channel. */
      std::shared_ptr<Message<ServiceProtocolClient>> ReadMessage();

      /** Spawns a Message handling loop for this ServiceProtocolClient. */
      void SpawnMessageHandler();

      void Close();

    private:
      mutable boost::mutex m_mutex;
      typename P::template apply<ServiceSlots>::type m_slots;
      MessageProtocol m_protocol;
      GetOptionalLocalPtr<T> m_timer;
      Session m_session;
      Routines::RoutineHandler m_readLoop;
      Routines::RoutineHandler m_timerLoop;
      std::shared_ptr<Queue<Threading::Timer::Result>> m_timerQueue;
      Routines::RoutineHandler m_messageHandler;
      std::atomic_int m_nextRequestId;
      std::unordered_map<int, Routines::BaseEval*> m_pendingRequests;
      Queue<std::shared_ptr<Message<ServiceProtocolClient>>> m_messages;
      std::atomic_bool m_isReading;
      IO::OpenState m_openState;

      ServiceProtocolClient(const ServiceProtocolClient&) = delete;
      ServiceProtocolClient& operator =(
        const ServiceProtocolClient&) = delete;
      void Open();
      void Shutdown();
      void ReadLoop();
      void TimerLoop();
  };

  /**
   * A type trait to determine whether a servlet handles parallel message
   * handling.
   * @param <T> The type of ServiceProtocolClient.
   */
  template<typename T, typename Enabled = void>
  struct SupportsParallelism {
    static constexpr auto value = false;
  };

  template<typename T>
  struct SupportsParallelism<T,
      std::enable_if_t<Details::HasParallelism<T>::value>> {
    static constexpr auto value = T::SupportsParallelism;
  };

  /**
   * Implements a basic Message handling loop for a ServiceProtocolClient.
   * @param client The ServiceProtocolClient to handle the Messages for.
   */
  template<typename ServiceProtocolClient>
  void HandleMessagesLoop(ServiceProtocolClient& client) {
    auto routines = Routines::RoutineHandlerGroup();
    try {
      while(true) {
        auto message = client.ReadMessage();
        if(auto slot = client.GetSlots().Find(*message)) {
          if constexpr(SupportsParallelism<ServiceProtocolClient>::value) {
            routines.Spawn(
              [&, message = std::move(message), slot = std::move(slot)] {
                try {
                  message->EmitSignal(slot, Ref(client));
                } catch(const std::exception&) {
                  client.Close();
                }
              });
          } else {
            try {
              message->EmitSignal(slot, Ref(client));
            } catch(const std::exception&) {
              client.Close();
            }
          }
        }
      }
    } catch(const IO::EndOfFileException&) {
      return;
    }
  }

  template<typename M, typename T, typename P, typename S, bool V>
  template<typename CF, typename SF, typename TF>
  ServiceProtocolClient<M, T, P, S, V>::ServiceProtocolClient(CF&& channel,
      SF&& slots, TF&& timer)
      : m_slots(std::forward<SF>(slots)),
        m_protocol(std::forward<CF>(channel), Ref(m_slots->GetRegistry()),
          Ref(m_slots->GetRegistry()), Initialize(), Initialize()),
        m_timer(std::forward<TF>(timer)),
        m_timerQueue(std::make_shared<Queue<Threading::Timer::Result>>()),
        m_nextRequestId(1),
        m_isReading(false) {
    m_timer->GetPublisher().Monitor(m_timerQueue);
  }

  template<typename M, typename T, typename P, typename S, bool V>
  template<typename CF, typename TF>
  ServiceProtocolClient<M, T, P, S, V>::ServiceProtocolClient(CF&& channel,
    TF&& timer)
    : ServiceProtocolClient(std::forward<CF>(channel), Initialize(),
        std::forward<TF>(timer)) {}

  template<typename M, typename T, typename P, typename S, bool V>
  ServiceProtocolClient<M, T, P, S, V>::~ServiceProtocolClient() {
    Close();
  }

  template<typename M, typename T, typename P, typename S, bool V>
  const typename ServiceProtocolClient<M, T, P, S, V>::ServiceSlots&
      ServiceProtocolClient<M, T, P, S, V>::GetSlots() const {
    return *m_slots;
  }

  template<typename M, typename T, typename P, typename S, bool V>
  typename ServiceProtocolClient<M, T, P, S, V>::ServiceSlots&
      ServiceProtocolClient<M, T, P, S, V>::GetSlots() {
    return *m_slots;
  }

  template<typename M, typename T, typename P, typename S, bool V>
  const typename ServiceProtocolClient<M, T, P, S, V>::Session&
      ServiceProtocolClient<M, T, P, S, V>::GetSession() const {
    return m_session;
  }

  template<typename M, typename T, typename P, typename S, bool V>
  typename ServiceProtocolClient<M, T, P, S, V>::Session&
      ServiceProtocolClient<M, T, P, S, V>::GetSession() {
    return m_session;
  }

  template<typename M, typename T, typename P, typename S, bool V>
  std::unique_ptr<ServiceRequestException> ServiceProtocolClient<
      M, T, P, S, V>::CloneException(const ServiceRequestException& e) {
    return m_protocol.Clone(e);
  }

  template<typename M, typename T, typename P, typename S, bool V>
  template<typename Buffer>
  void ServiceProtocolClient<M, T, P, S, V>::Encode(
      const Message<ServiceProtocolClient>& message, Out<Buffer> buffer) {
    m_protocol.Encode(&message, Store(buffer));
  }

  template<typename M, typename T, typename P, typename S, bool V>
  void ServiceProtocolClient<M, T, P, S, V>::Send(
      const Message<ServiceProtocolClient>& message) {
    m_protocol.Send(&message);
  }

  template<typename M, typename T, typename P, typename S, bool V>
  template<typename Buffer, typename>
  void ServiceProtocolClient<M, T, P, S, V>::Send(const Buffer& buffer) {
    m_protocol.Send(buffer);
  }

  template<typename M, typename T, typename P, typename S, bool V>
  template<typename Service>
  GetStorageType<typename Service::Return>
      ServiceProtocolClient<M, T, P, S, V>::SendServiceRequest(
      const typename Service::Parameters& parameters) {
    auto resultAsync = Routines::Async<typename Service::Return>();
    auto resultEval = resultAsync.GetEval();
    auto requestId = ++m_nextRequestId;
    auto request = typename Service::template Request<ServiceProtocolClient>(
      requestId, parameters);
    {
      auto lock = boost::lock_guard(m_mutex);
      m_pendingRequests.insert(std::pair(requestId, &resultEval));
    }
    Open();
    try {
      m_protocol.Send(&request);
    } catch(const std::exception&) {
      auto lock = boost::lock_guard(m_mutex);
      m_pendingRequests.erase(requestId);
      BOOST_RETHROW;
    }
    return std::move(resultAsync.Get());
  }

  template<typename M, typename T, typename P, typename S, bool V>
  template<typename Service, typename... Args>
  GetStorageType<typename Service::Return>
      ServiceProtocolClient<M, T, P, S, V>::SendRequest(Args&&... args) {
    return SendServiceRequest<Service>(
      typename Service::Parameters(std::forward<Args>(args)...));
  }

  template<typename M, typename T, typename P, typename S, bool V>
  std::shared_ptr<Message<ServiceProtocolClient<M, T, P, S, V>>>
      ServiceProtocolClient<M, T, P, S, V>::ReadMessage() {
    Open();
    return m_messages.Pop();
  }

  template<typename M, typename T, typename P, typename S, bool V>
  void ServiceProtocolClient<M, T, P, S, V>::SpawnMessageHandler() {
    m_messageHandler = Routines::Spawn(
      std::bind(HandleMessagesLoop<ServiceProtocolClient>, std::ref(*this)));
  }

  template<typename M, typename T, typename P, typename S, bool V>
  void ServiceProtocolClient<M, T, P, S, V>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
    m_readLoop.Wait();
    m_messageHandler.Wait();
    m_timerLoop.Wait();
    m_openState.Close();
  }

  template<typename M, typename T, typename P, typename S, bool V>
  void ServiceProtocolClient<M, T, P, S, V>::Open() {
    if(m_isReading.exchange(true)) {
      return;
    }
    m_timer->Start();
    m_timerLoop = Routines::Spawn(
      std::bind(&ServiceProtocolClient::TimerLoop, this));
    m_readLoop = Routines::Spawn(
      std::bind(&ServiceProtocolClient::ReadLoop, this));
  }

  template<typename M, typename T, typename P, typename S, bool V>
  void ServiceProtocolClient<M, T, P, S, V>::Shutdown() {
    m_protocol.Close();
    m_messages.Break(IO::EndOfFileException());
    m_timer->Cancel();
    auto pendingRequests = std::unordered_map<int, Routines::BaseEval*>();
    {
      auto lock = boost::lock_guard(m_mutex);
      pendingRequests.swap(m_pendingRequests);
    }
    for(auto& eval : pendingRequests | boost::adaptors::map_values) {
      eval->SetException(ServiceRequestException(
        "ServiceProtocolClient closed."));
    }
  }

  template<typename M, typename T, typename P, typename S, bool V>
  void ServiceProtocolClient<M, T, P, S, V>::ReadLoop() {
    while(true) {
      auto message = std::unique_ptr<Message<ServiceProtocolClient>>();
      try {
        message = m_protocol.template Receive<
          std::unique_ptr<Message<ServiceProtocolClient>>>();
        if(!message) {
          return;
        }
      } catch(const IO::EndOfFileException&) {
        Shutdown();
        return;
      } catch(const std::exception&) {
        std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        Shutdown();
        return;
      }
      auto serviceMessage =
        dynamic_cast<ServiceMessage<ServiceProtocolClient>*>(message.get());
      if(serviceMessage != nullptr && serviceMessage->IsResponseMessage()) {
        auto eval = [&] {
          auto lock = boost::lock_guard(m_mutex);
          auto responseIterator = m_pendingRequests.find(
            serviceMessage->GetRequestId());
          if(responseIterator != m_pendingRequests.end()) {
            auto eval = responseIterator->second;
            m_pendingRequests.erase(responseIterator);
            return eval;
          }
          return static_cast<Routines::BaseEval*>(nullptr);
        }();
        if(eval) {
          serviceMessage->SetEval(*eval);
        }
      } else {
        try {
          m_messages.Push(std::move(message));
        } catch(const IO::EndOfFileException&) {
          Shutdown();
          return;
        } catch(const std::exception&) {
          std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
          Shutdown();
          return;
        }
      }
    }
  }

  template<typename M, typename T, typename P, typename S, bool V>
  void ServiceProtocolClient<M, T, P, S, V>::TimerLoop() {
    auto heartbeatMessage = HeartbeatMessage<ServiceProtocolClient>();
    try {
      while(m_openState.IsOpen()) {
        if(m_timerQueue->Pop() == Threading::Timer::Result::EXPIRED) {
          Send(heartbeatMessage);
        } else {
          break;
        }
        m_timer->Start();
      }
    } catch(const PipeBrokenException&) {
      return;
    }
  }
}

#endif
