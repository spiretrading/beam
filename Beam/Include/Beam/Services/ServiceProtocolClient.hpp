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
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPointerPolicy.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Ref.hpp"
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
#include "Beam/Services/NullSession.hpp"
#include "Beam/Services/RecordMessage.hpp"
#include "Beam/Services/Service.hpp"
#include "Beam/Services/ServiceRequestException.hpp"
#include "Beam/Services/ServiceSlots.hpp"
#include "Beam/TimeService/Timer.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {

  /**
   * Implements the service protocol on top of a Channel.
   * @param M The type of MessageProtocol used to send and receive messages.
   * @param T The type of Timer used for heartbeats.
   * @param P The pointer policy used for ServiceSlots.
   * @param S Stores session information.
   * @param V Whether this client supports handling messages in parallel.
   */
  template<typename M, typename T, typename P = LocalPointerPolicy,
    typename S = NullSession, bool V = false> requires IsTimer<dereference_t<T>>
  class ServiceProtocolClient {
    public:

      /** The type of MessageProtocol used to send and receive messages. */
      using MessageProtocol = M;

      /** The type of ServiceSlots used. */
      using ServiceSlots = Beam::ServiceSlots<ServiceProtocolClient>;

      /** The type of Timer used. */
      using Timer = T;

      /** The type of Session info stored. */
      using Session = S;

      /** Whether this client supports handling messages in parallel. */
      static constexpr auto SUPPORTS_PARALLELISM = V;

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
      const ServiceSlots& get_slots() const;

      /** Returns the ServiceSlots. */
      ServiceSlots& get_slots();

      /** Returns the session info. */
      const Session& get_session() const;

      /** Returns the session info. */
      Session& get_session();

      /**
       * Clones a ServiceRequestException usable with this protocol.
       * @param e The ServiceRequestException to clone.
       */
      std::unique_ptr<ServiceRequestException> clone_exception(
        const ServiceRequestException& e);

      /**
       * Encodes a Message using this client's MessageProtocol.
       * @param message The Message to encode.
       * @param buffer The Buffer to store the encoded Message in.
       */
      template<IsBuffer B>
      void encode(const Message<ServiceProtocolClient>& message, Out<B> buffer);

      /**
       * Sends a Message.
       * @param message The Message to send.
       */
      void send(const Message<ServiceProtocolClient>& message);

      /**
       * Sends a Buffer.
       * @param buffer The Buffer to send.
       */
      template<IsConstBuffer B>
      void send(const B& buffer);

      /**
       * Sends a request for a Service.
       * @param parameters The Service's parameters.
       * @return The response to this Service::Request.
       */
      template<typename Service>
      typename Service::Return send_service_request(
        const typename Service::Parameters& parameters);

      /**
       * Sends a request for a Service.
       * @param args The parameters to send.
       * @return The response to this Service::Request.
       */
      template<typename Service, typename... Args>
      typename Service::Return send_request(Args&&... args);

      /** Reads a Message from the Channel. */
      std::shared_ptr<Message<ServiceProtocolClient>> read_message();

      /** Spawns a Message handling loop for this ServiceProtocolClient. */
      void spawn_message_handler();

      void close();

    private:
      mutable boost::mutex m_mutex;
      Mutex m_read_mutex;
      typename P::template apply<ServiceSlots>::type m_slots;
      MessageProtocol m_protocol;
      local_ptr_t<T> m_timer;
      Session m_session;
      RoutineHandler m_read_loop;
      RoutineHandler m_timer_loop;
      std::shared_ptr<Queue<Beam::Timer::Result>> m_timer_queue;
      RoutineHandler m_message_handler;
      std::atomic_int m_next_request_id;
      std::unordered_map<int, BaseEval*> m_pending_requests;
      Queue<std::shared_ptr<Message<ServiceProtocolClient>>> m_messages;
      std::atomic_bool m_is_reading;
      OpenState m_open_state;

      ServiceProtocolClient(const ServiceProtocolClient&) = delete;
      ServiceProtocolClient& operator =(const ServiceProtocolClient&) = delete;
      void open();
      void shutdown();
      void read_loop();
      void timer_loop();
  };

  /**
   * A type trait to determine whether a servlet handles parallel message
   * handling.
   * @tparam T The type of ServiceProtocolClient.
   */
  template<typename T>
  struct supports_parallelism {
    static constexpr auto value = false;
  };

  template<typename T>
  constexpr auto supports_parallelism_v = supports_parallelism<T>::value;

  template<typename T> requires requires { T::SUPPORTS_PARALLELISM; }
  struct supports_parallelism<T> {
    static constexpr auto value = T::SUPPORTS_PARALLELISM;
  };

  /**
   * Implements a basic Message handling loop for a ServiceProtocolClient.
   * @param client The ServiceProtocolClient to handle the Messages for.
   */
  template<typename ServiceProtocolClient>
  void handle_messages_loop(ServiceProtocolClient& client) {
    auto routines = RoutineHandlerGroup();
    try {
      while(true) {
        auto message = client.read_message();
        if(auto slot = client.get_slots().find(*message)) {
          if constexpr(supports_parallelism_v<ServiceProtocolClient>) {
            routines.spawn(
              [&, message = std::move(message), slot = std::move(slot)] {
                try {
                  message->emit(slot, Ref(client));
                } catch(const std::exception&) {
                  client.close();
                }
              });
          } else {
            try {
              message->emit(slot, Ref(client));
            } catch(const std::exception&) {
              client.close();
            }
          }
        }
      }
    } catch(const EndOfFileException&) {
      return;
    }
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  template<typename CF, typename SF, typename TF>
  ServiceProtocolClient<M, T, P, S, V>::ServiceProtocolClient(
      CF&& channel, SF&& slots, TF&& timer)
      : m_slots(std::forward<SF>(slots)),
        m_protocol(std::forward<CF>(channel), Ref(m_slots->get_registry()),
          Ref(m_slots->get_registry()), typename MessageProtocol::Encoder(),
          typename MessageProtocol::Decoder()),
        m_timer(std::forward<TF>(timer)),
        m_timer_queue(std::make_shared<Queue<Beam::Timer::Result>>()),
        m_next_request_id(1),
        m_is_reading(false) {
    m_timer->get_publisher().monitor(m_timer_queue);
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  template<typename CF, typename TF>
  ServiceProtocolClient<M, T, P, S, V>::ServiceProtocolClient(
    CF&& channel, TF&& timer)
    : ServiceProtocolClient(
        std::forward<CF>(channel), init(), std::forward<TF>(timer)) {}

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  ServiceProtocolClient<M, T, P, S, V>::~ServiceProtocolClient() {
    close();
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  const typename ServiceProtocolClient<M, T, P, S, V>::ServiceSlots&
      ServiceProtocolClient<M, T, P, S, V>::get_slots() const {
    return *m_slots;
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  typename ServiceProtocolClient<M, T, P, S, V>::ServiceSlots&
      ServiceProtocolClient<M, T, P, S, V>::get_slots() {
    return const_cast<ServiceSlots&>(*m_slots);
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  const typename ServiceProtocolClient<M, T, P, S, V>::Session&
      ServiceProtocolClient<M, T, P, S, V>::get_session() const {
    return m_session;
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  typename ServiceProtocolClient<M, T, P, S, V>::Session&
      ServiceProtocolClient<M, T, P, S, V>::get_session() {
    return m_session;
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  std::unique_ptr<ServiceRequestException> ServiceProtocolClient<
      M, T, P, S, V>::clone_exception(const ServiceRequestException& e) {
    return m_protocol.clone(e);
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  template<IsBuffer B>
  void ServiceProtocolClient<M, T, P, S, V>::encode(
      const Message<ServiceProtocolClient>& message, Out<B> buffer) {
    m_protocol.encode(message, out(buffer));
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  void ServiceProtocolClient<M, T, P, S, V>::send(
      const Message<ServiceProtocolClient>& message) {
    m_protocol.send(&message);
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  template<IsConstBuffer B>
  void ServiceProtocolClient<M, T, P, S, V>::send(const B& buffer) {
    m_protocol.send(buffer);
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  template<typename Service>
  typename Service::Return
    ServiceProtocolClient<M, T, P, S, V>::send_service_request(
      const typename Service::Parameters& parameters) {
    auto result_async = Async<typename Service::Return>();
    auto result_eval = result_async.get_eval();
    auto request_id = ++m_next_request_id;
    auto request = typename Service::template Request<ServiceProtocolClient>(
      request_id, parameters);
    {
      auto lock = boost::lock_guard(m_mutex);
      m_pending_requests.insert(std::pair(request_id, &result_eval));
    }
    open();
    try {
      m_protocol.send(&request);
    } catch(const std::exception&) {
      auto lock = boost::lock_guard(m_mutex);
      m_pending_requests.erase(request_id);
      throw;
    }
    if constexpr(std::same_as<typename Service::Return, void>) {
      result_async.get();
    } else {
      return std::move(result_async.get());
    }
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  template<typename Service, typename... Args>
  typename Service::Return
      ServiceProtocolClient<M, T, P, S, V>::send_request(Args&&... args) {
    return send_service_request<Service>(
      typename Service::Parameters(std::forward<Args>(args)...));
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  std::shared_ptr<Message<ServiceProtocolClient<M, T, P, S, V>>>
      ServiceProtocolClient<M, T, P, S, V>::read_message() {
    open();
    return m_messages.pop();
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  void ServiceProtocolClient<M, T, P, S, V>::spawn_message_handler() {
    m_message_handler = spawn(std::bind_front(
      handle_messages_loop<ServiceProtocolClient>, std::ref(*this)));
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  void ServiceProtocolClient<M, T, P, S, V>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    shutdown();
    {
      auto lock = boost::lock_guard(m_read_mutex);
      m_read_loop.wait();
    }
    m_message_handler.wait();
    m_timer_loop.wait();
    m_open_state.close();
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  void ServiceProtocolClient<M, T, P, S, V>::open() {
    if(m_is_reading.exchange(true)) {
      return;
    }
    m_timer->start();
    m_timer_loop =
      spawn(std::bind_front(&ServiceProtocolClient::timer_loop, this));
    auto lock = boost::lock_guard(m_read_mutex);
    m_read_loop =
      spawn(std::bind_front(&ServiceProtocolClient::read_loop, this));
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  void ServiceProtocolClient<M, T, P, S, V>::shutdown() {
    m_is_reading = true;
    m_protocol.close();
    m_messages.close(EndOfFileException());
    m_timer->cancel();
    auto pending_requests = std::unordered_map<int, BaseEval*>();
    {
      auto lock = boost::lock_guard(m_mutex);
      pending_requests.swap(m_pending_requests);
    }
    for(auto& eval : pending_requests | boost::adaptors::map_values) {
      eval->set_exception(
        ServiceRequestException("ServiceProtocolClient closed."));
    }
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  void ServiceProtocolClient<M, T, P, S, V>::read_loop() {
    while(true) {
      auto message = std::unique_ptr<Message<ServiceProtocolClient>>();
      try {
        message = m_protocol.
          template receive<std::unique_ptr<Message<ServiceProtocolClient>>>();
        if(!message) {
          return;
        }
      } catch(const EndOfFileException&) {
        shutdown();
        return;
      } catch(const std::exception&) {
        std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        shutdown();
        return;
      }
      auto service_message =
        dynamic_cast<ServiceMessage<ServiceProtocolClient>*>(message.get());
      if(service_message && service_message->is_response()) {
        auto eval = [&] {
          auto lock = boost::lock_guard(m_mutex);
          auto i = m_pending_requests.find(service_message->get_id());
          if(i != m_pending_requests.end()) {
            auto eval = i->second;
            m_pending_requests.erase(i);
            return eval;
          }
          return static_cast<BaseEval*>(nullptr);
        }();
        if(eval) {
          service_message->set_eval(*eval);
        }
      } else {
        try {
          m_messages.push(std::move(message));
        } catch(const EndOfFileException&) {
          shutdown();
          return;
        } catch(const std::exception&) {
          std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
          shutdown();
          return;
        }
      }
    }
  }

  template<typename M, typename T, typename P, typename S, bool V> requires
    IsTimer<dereference_t<T>>
  void ServiceProtocolClient<M, T, P, S, V>::timer_loop() {
    auto heartbeat_message = HeartbeatMessage<ServiceProtocolClient>();
    try {
      while(m_open_state.is_open()) {
        if(m_timer_queue->pop() == Beam::Timer::Result::EXPIRED) {
          send(heartbeat_message);
        } else {
          break;
        }
        m_timer->start();
      }
    } catch(const PipeBrokenException&) {
      return;
    } catch(const EndOfFileException&) {
      return;
    } catch(const std::exception&) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
    }
  }
}

#endif
