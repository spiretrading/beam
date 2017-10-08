#ifndef BEAM_SERVICEPROTOCOLCLIENT_HPP
#define BEAM_SERVICEPROTOCOLCLIENT_HPP
#include <iostream>
#include <unordered_map>
#include <boost/atomic.hpp>
#include <boost/noncopyable.hpp>
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
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/NullType.hpp"
#include "Beam/Utilities/ReportException.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"

namespace Beam {
namespace Services {
namespace Details {
  BEAM_DEFINE_HAS_VARIABLE(HasParallelism, SupportsParallelism);
}

  /*! \class ServiceProtocolClient
      \brief Implements the service protocol on top of a Channel.
      \tparam MessageProtocolType The type of MessageProtocol used to send and
              receive messages.
      \tparam TimerType The type of Timer used for heartbeats.
      \tparam ServiceSlotsPolicy The pointer policy used for ServiceSlots.
      \tparam SessionType Stores session information.
      \tparam SupportsParallelismValue Whether this client supports handling
              messages in parallel.
   */
  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy = LocalPointerPolicy,
    typename SessionType = NullType, bool SupportsParallelismValue = false>
  class ServiceProtocolClient : private boost::noncopyable {
    public:

      //! The type of MessageProtocol used to send and receive messages.
      using MessageProtocol = MessageProtocolType;

      //! The type of ServiceSlots used.
      using ServiceSlots = Services::ServiceSlots<ServiceProtocolClient>;

      //! The type of Timer used.
      using Timer = TimerType;

      //! The type of Session info stored.
      using Session = SessionType;

      //! Whether this client supports handling messages in parallel.
      static constexpr bool SupportsParallelism = SupportsParallelismValue;

      //! Constructs a ServiceProtocolClient.
      /*!
        \param channel Initializes the client's Channel.
        \param slots Initializes the ServiceSlots.
        \param timer The type of Timer used.
      */
      template<typename ChannelForward, typename ServiceSlotsForward,
        typename TimerForward>
      ServiceProtocolClient(ChannelForward&& channel,
        ServiceSlotsForward&& slots, TimerForward&& timer);

      //! Constructs a ServiceProtocolClient.
      /*!
        \param channel Initializes the client's Channel.
        \param timer The type of Timer used.
      */
      template<typename ChannelForward, typename TimerForward>
      ServiceProtocolClient(ChannelForward&& channel, TimerForward&& timer);

      ~ServiceProtocolClient();

      //! Returns the ServiceSlots.
      const ServiceSlots& GetSlots() const;

      //! Returns the ServiceSlots.
      ServiceSlots& GetSlots();

      //! Returns the session info.
      const Session& GetSession() const;

      //! Returns the session info.
      Session& GetSession();

      //! Clones a ServiceRequestException usable with this protocol.
      /*!
        \param e The ServiceRequestException to clone.
      */
      std::unique_ptr<ServiceRequestException> CloneException(
        const ServiceRequestException& e);

      //! Encodes a Message using this client's MessageProtocol.
      /*!
        \param message The Message to encode.
        \param buffer The Buffer to store the encoded Message in.
      */
      template<typename Buffer>
      void Encode(const Message<ServiceProtocolClient>& message,
        Out<Buffer> buffer);

      //! Sends a Message.
      /*!
        \param message The Message to send.
      */
      void Send(const Message<ServiceProtocolClient>& message);

      //! Sends a Buffer.
      /*!
        \param buffer The Buffer to send.
      */
      template<typename Buffer>
      typename std::enable_if<ImplementsConcept<
        Buffer, IO::Buffer>::value>::type Send(const Buffer& buffer);

      //! Sends a request for a Service.
      /*!
        \param parameters The Service's parameters.
        \return The response to this Service::Request.
      */
      template<typename Service>
      GetStorageType<typename Service::Return> SendServiceRequest(
        const typename Service::Parameters& parameters);

      //! Sends a request for a Service.
      /*!
        \param args The parameters to send.
        \return The response to this Service::Request.
      */
      template<typename Service, typename... Args>
      GetStorageType<typename Service::Return> SendRequest(Args&&... args);

      //! Reads a Message from the Channel.
      std::shared_ptr<Message<ServiceProtocolClient>> ReadMessage();

      //! Spawns a Message handling loop for this ServiceProtocolClient.
      void SpawnMessageHandler();

      void Open();

      void Close();

    private:
      mutable boost::mutex m_mutex;
      typename ServiceSlotsPolicy::template apply<ServiceSlots>::type m_slots;
      MessageProtocol m_protocol;
      GetOptionalLocalPtr<TimerType> m_timer;
      Session m_session;
      Routines::RoutineHandler m_readLoop;
      Routines::RoutineHandler m_timerLoop;
      std::shared_ptr<Queue<Threading::Timer::Result>> m_timerQueue;
      Routines::RoutineHandler m_messageHandler;
      boost::atomic_int m_nextRequestId;
      std::unordered_map<int, Routines::BaseEval*> m_pendingRequests;
      Queue<std::shared_ptr<Message<ServiceProtocolClient>>> m_messages;
      bool m_isShuttingDown;
      IO::OpenState m_openState;

      void Shutdown();
      void Fail(void* source);
      void ReadLoop();
      void TimerLoop();
  };

  /*! \class SupportsParallelism
      \brief A type trait to determine whether a servlet handles parallel
             message handling.
      \tparam T The type of ServiceProtocolClient.
   */
  template<typename T, typename Enabled = void>
  struct SupportsParallelism {
    static constexpr bool value = false;
  };

  template<typename T>
  struct SupportsParallelism<T, typename std::enable_if<
      Details::HasParallelism<T>::value>::type> {
    static constexpr bool value = T::SupportsParallelism;
  };

  //! Implements a basic Message handling loop for a ServiceProtocolClient.
  /*!
    \param client The ServiceProtocolClient to handle the Messages for.
  */
  template<typename ServiceProtocolClientType>
  void HandleMessagesLoop(ServiceProtocolClientType& client) {
    Routines::RoutineHandlerGroup routines;
    try {
      while(true) {
        auto message = client.ReadMessage();
        auto slot = client.GetSlots().Find(*message);
        if(slot != nullptr) {
          if(SupportsParallelism<ServiceProtocolClientType>::value) {
            routines.Spawn(
              [&, message = std::move(message), slot = std::move(slot)] {
                try {
                  message->EmitSignal(slot, Ref(client));
                } catch(...) {
                  client.Close();
                }
              });
          } else {
            try {
              message->EmitSignal(slot, Ref(client));
            } catch(...) {
              client.Close();
            }
          }
        }
      }
    } catch(const IO::EndOfFileException&) {
      return;
    }
  };

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  template<typename ChannelForward, typename ServiceSlotsForward,
    typename TimerForward>
  ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::ServiceProtocolClient(
      ChannelForward&& channel, ServiceSlotsForward&& slots,
      TimerForward&& timer)
      : m_slots(std::forward<ServiceSlotsForward>(slots)),
        m_protocol(std::forward<ChannelForward>(channel),
          Ref(m_slots->GetRegistry()), Ref(m_slots->GetRegistry()),
          Initialize(), Initialize()),
        m_timer(std::forward<TimerForward>(timer)),
        m_nextRequestId(1),
        m_isShuttingDown(false) {}

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  template<typename ChannelForward, typename TimerForward>
  ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::ServiceProtocolClient(
      ChannelForward&& channel, TimerForward&& timer)
      : m_protocol(std::forward<ChannelForward>(channel),
          Ref(m_slots->GetRegistry()), Ref(m_slots->GetRegistry()),
          Initialize(), Initialize()),
        m_timer(std::forward<TimerForward>(timer)),
        m_nextRequestId(1),
        m_isShuttingDown(false) {}

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::~ServiceProtocolClient() {
    Close();
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  const typename ServiceProtocolClient<MessageProtocolType, TimerType,
      ServiceSlotsPolicy, SessionType, SupportsParallelismValue>::ServiceSlots&
      ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::GetSlots() const {
    return *m_slots;
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  typename ServiceProtocolClient<MessageProtocolType, TimerType,
      ServiceSlotsPolicy, SessionType, SupportsParallelismValue>::ServiceSlots&
      ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::GetSlots() {
    return *m_slots;
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  const typename ServiceProtocolClient<MessageProtocolType, TimerType,
      ServiceSlotsPolicy, SessionType, SupportsParallelismValue>::Session&
      ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::GetSession() const {
    return m_session;
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  typename ServiceProtocolClient<MessageProtocolType, TimerType,
      ServiceSlotsPolicy, SessionType, SupportsParallelismValue>::Session&
      ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::GetSession() {
    return m_session;
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  std::unique_ptr<ServiceRequestException> ServiceProtocolClient<
      MessageProtocolType, TimerType, ServiceSlotsPolicy, SessionType,
      SupportsParallelismValue>::CloneException(
      const ServiceRequestException& e) {
    return m_protocol.Clone(e);
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  template<typename Buffer>
  void ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::Encode(
      const Message<ServiceProtocolClient>& message, Out<Buffer> buffer) {
    m_protocol.Encode(&message, Store(buffer));
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  void ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::Send(
      const Message<ServiceProtocolClient>& message) {
    m_protocol.Send(&message);
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  template<typename Buffer>
  typename std::enable_if<ImplementsConcept<
      Buffer, IO::Buffer>::value>::type ServiceProtocolClient<
      MessageProtocolType, TimerType, ServiceSlotsPolicy, SessionType,
      SupportsParallelismValue>::Send(const Buffer& buffer) {
    m_protocol.Send(buffer);
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  template<typename Service>
  GetStorageType<typename Service::Return> ServiceProtocolClient<
      MessageProtocolType, TimerType, ServiceSlotsPolicy, SessionType,
      SupportsParallelismValue>::SendServiceRequest(
      const typename Service::Parameters& parameters) {
    Routines::Async<typename Service::Return> resultAsync;
    auto resultEval = resultAsync.GetEval();
    auto requestId = ++m_nextRequestId;
    typename Service::template Request<ServiceProtocolClient> request(requestId,
      parameters);
    {
      boost::lock_guard<boost::mutex> lock(m_mutex);
      if(!m_openState.IsOpen() || m_isShuttingDown) {
        BOOST_THROW_EXCEPTION(ServiceRequestException(
          "ServiceProtocolClient closed."));
      }
      m_pendingRequests.insert(std::make_pair(requestId, &resultEval));
    }
    try {
      m_protocol.Send(&request);
    } catch(const std::exception&) {
      boost::lock_guard<boost::mutex> lock(m_mutex);
      m_pendingRequests.erase(requestId);
      BOOST_RETHROW;
    }
    return std::move(resultAsync.Get());
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  template<typename Service, typename... Args>
  GetStorageType<typename Service::Return> ServiceProtocolClient<
      MessageProtocolType, TimerType, ServiceSlotsPolicy, SessionType,
      SupportsParallelismValue>::SendRequest(Args&&... args) {
    return SendServiceRequest<Service>(
      typename Service::Parameters(std::forward<Args>(args)...));
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  std::shared_ptr<Message<ServiceProtocolClient<MessageProtocolType, TimerType,
      ServiceSlotsPolicy, SessionType, SupportsParallelismValue>>>
      ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::ReadMessage() {
    auto message = m_messages.Top();
    m_messages.Pop();
    return message;
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  void ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::SpawnMessageHandler() {
    m_messageHandler = Routines::Spawn(
      std::bind(HandleMessagesLoop<ServiceProtocolClient>, std::ref(*this)));
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  void ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    {
      boost::lock_guard<boost::mutex> lock(m_mutex);
      m_isShuttingDown = false;
    }
    try {
      m_protocol.GetChannel().GetConnection().Open();
      m_timerQueue = std::make_shared<Queue<Threading::Timer::Result>>();
      m_timer->GetPublisher().Monitor(m_timerQueue);
      m_timer->Start();
      m_timerLoop = Routines::Spawn(
        std::bind(&ServiceProtocolClient::TimerLoop, this));
      m_readLoop = Routines::Spawn(
        std::bind(&ServiceProtocolClient::ReadLoop, this));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  void ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  void ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::Shutdown() {
    Fail(nullptr);
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  void ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::Fail(void* source) {
    {
      boost::lock_guard<boost::mutex> lock(m_mutex);
      m_isShuttingDown = true;
    }
    m_protocol.GetChannel().GetConnection().Close();
    m_messages.Break(IO::EndOfFileException());
    if(source != &m_timerLoop) {
      m_timer->Cancel();
      m_timerLoop.Wait();
    }
    if(source != &m_readLoop) {
      m_readLoop.Wait();
    }
    m_messageHandler.Wait();
    std::unordered_map<int, Routines::BaseEval*> pendingRequests;
    {
      boost::lock_guard<boost::mutex> lock(m_mutex);
      pendingRequests.swap(m_pendingRequests);
    }
    for(auto& eval : pendingRequests | boost::adaptors::map_values) {
      eval->SetException(ServiceRequestException(
        "ServiceProtocolClient closed."));
    }
    m_openState.SetClosed();
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  void ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::ReadLoop() {
    while(true) {
      std::unique_ptr<Message<ServiceProtocolClient>> message;
      try {
        message = m_protocol.template Receive<
          std::unique_ptr<Message<ServiceProtocolClient>>>();
        if(message == nullptr) {
          Fail(&m_readLoop);
          return;
        }
      } catch(const IO::EndOfFileException&) {
        Fail(&m_readLoop);
        return;
      } catch(const std::exception&) {
        std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        Fail(&m_readLoop);
        return;
      }
      auto serviceMessage =
        dynamic_cast<ServiceMessage<ServiceProtocolClient>*>(message.get());
      if(serviceMessage != nullptr && serviceMessage->IsResponseMessage()) {
        Routines::BaseEval* eval;
        {
          boost::lock_guard<boost::mutex> lock(m_mutex);
          auto responseIterator = m_pendingRequests.find(
            serviceMessage->GetRequestId());
          if(responseIterator != m_pendingRequests.end()) {
            eval = responseIterator->second;
            m_pendingRequests.erase(responseIterator);
          } else {
            eval = nullptr;
          }
        }
        if(eval != nullptr) {
          serviceMessage->SetEval(*eval);
        }
      } else {
        try {
          m_messages.Push(std::move(message));
        } catch(const IO::EndOfFileException&) {
          Fail(&m_readLoop);
          return;
        } catch(const std::exception&) {
          std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
          Fail(&m_readLoop);
          return;
        }
      }
    }
  }

  template<typename MessageProtocolType, typename TimerType,
    typename ServiceSlotsPolicy, typename SessionType,
    bool SupportsParallelismValue>
  void ServiceProtocolClient<MessageProtocolType, TimerType, ServiceSlotsPolicy,
      SessionType, SupportsParallelismValue>::TimerLoop() {
    HeartbeatMessage<ServiceProtocolClient> heartbeatMessage;
    try {
      while(m_openState.IsRunning()) {
        auto result = m_timerQueue->Top();
        m_timerQueue->Pop();
        if(result == Threading::Timer::Result::EXPIRED) {
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
}

#endif
