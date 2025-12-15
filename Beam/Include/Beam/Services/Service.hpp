#ifndef BEAM_SERVICE_HPP
#define BEAM_SERVICE_HPP
#include <concepts>
#include <functional>
#include <vector>
#include <boost/pfr.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/comparison/greater.hpp>
#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/enum.hpp>
#include <boost/preprocessor/tuple/pop_front.hpp>
#include <boost/preprocessor/tuple/size.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include "Beam/Pointers/Out.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Serialization/ShuttleRecord.hpp"
#include "Beam/Services/Message.hpp"
#include "Beam/Services/RequestToken.hpp"
#include "Beam/Services/ServiceRequestException.hpp"
#include "Beam/Services/ServiceSlots.hpp"

#define BEAM_SERVICE_PARAMETERS_NAME(service)                                  \
  BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(0, service), Parameters)

#define BEAM_SERVICE_HAS_PARAMETERS(service)                                   \
  BOOST_PP_GREATER(BOOST_PP_TUPLE_SIZE(service), 3)

#define BEAM_DEFINE_EMPTY_SERVICE(service)                                     \
  namespace Details {                                                          \
    BEAM_DEFINE_RECORD(BEAM_SERVICE_PARAMETERS_NAME(service));                 \
  }

#define BEAM_DEFINE_PARAMETRIC_SERVICE(service)                                \
  namespace Details {                                                          \
    BEAM_DEFINE_RECORD(BEAM_SERVICE_PARAMETERS_NAME(service),                  \
      BOOST_PP_TUPLE_ENUM(                                                     \
        BOOST_PP_TUPLE_POP_FRONT(                                              \
          BOOST_PP_TUPLE_POP_FRONT(                                            \
            BOOST_PP_TUPLE_POP_FRONT(service)))))                              \
  }

#define BEAM_DEFINE_SERVICE(r, data, service)                                  \
  BOOST_PP_IIF(BEAM_SERVICE_HAS_PARAMETERS(service),                           \
    BEAM_DEFINE_PARAMETRIC_SERVICE, BEAM_DEFINE_EMPTY_SERVICE)(service)        \
                                                                               \
  using BOOST_PP_TUPLE_ELEM(0, service) =                                      \
    ::Beam::Service<BOOST_PP_TUPLE_ELEM(2, service),                           \
      Details::BEAM_SERVICE_PARAMETERS_NAME(service)>;

#define BEAM_REGISTER_SERVICE(r, data, service)                                \
  slots->get_registry().template add<                                          \
    BOOST_PP_TUPLE_ELEM(0, service)::Request<C>>(                              \
      BOOST_PP_TUPLE_ELEM(1, service) ".Request");                             \
  slots->get_registry().template add<                                          \
    BOOST_PP_TUPLE_ELEM(0, service)::Response<C>>(                             \
      BOOST_PP_TUPLE_ELEM(1, service) ".Response");

#define BEAM_DEFINE_SERVICES(name, ...)                                        \
  BOOST_PP_SEQ_FOR_EACH(                                                       \
    BEAM_DEFINE_SERVICE, *, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))             \
                                                                               \
  template<typename C>                                                         \
  void BOOST_PP_CAT(register_, name)(                                          \
      ::Beam::Out<::Beam::ServiceSlots<C>> slots) {                            \
    BOOST_PP_SEQ_FOR_EACH(                                                     \
      BEAM_REGISTER_SERVICE, *, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))         \
  }

namespace Beam {
namespace Details {
  template<typename, typename, typename>
  struct request_to_function;

  template<typename R, typename F, typename... Args>
  struct request_to_function<R, F, std::tuple<Args...>> {
    using type = std::function<R (F&, Args...)>;
  };

  template<typename C, typename S>
  struct request_slot_type {
    using type = typename request_to_function<void, RequestToken<C, S>,
      decltype(boost::pfr::structure_to_tuple(
        std::declval<typename S::Parameters>()))>::type;
  };

  template<typename C, typename S>
  using request_slot_t = typename request_slot_type<C, S>::type;

  template<typename C, typename S>
  struct slot_type {
    using type = typename request_to_function<typename S::Return, C,
      decltype(boost::pfr::structure_to_tuple(
        std::declval<typename S::Parameters>()))>::type;
  };

  template<typename C, typename S>
  using slot_t = typename slot_type<C, S>::type;

  template<typename R>
  class ServiceRequestSlot : public ServiceSlot<R> {
    public:
      using Request = R;
      using PreHook = typename ServiceSlot<Request>::PreHook;

      virtual void invoke(
        int id, Ref<typename Request::ServiceProtocolClient> client,
        const typename Request::Parameters& parameters) const = 0;
  };

  template<typename S, typename C>
  class ServiceRequestSlotImplementation final :
      public ServiceRequestSlot<typename S::template Request<C>> {
    public:
      using Service = S;
      using ServiceProtocolClient = C;
      using Request = typename Service::template Request<ServiceProtocolClient>;
      using Response =
        typename Service::template Response<ServiceProtocolClient>;
      using Slot = request_slot_t<C, Service>;
      using PreHook =
        typename ServiceRequestSlot<typename S::template Request<C>>::PreHook;

      template<typename F>
      explicit ServiceRequestSlotImplementation(F&& slot);

      void invoke(int id, Ref<ServiceProtocolClient> client,
        const typename Request::Parameters& parameters) const override;
      void add_pre_hook(const PreHook& hook) override;

    private:
      std::vector<PreHook> m_pre_hooks;
      Slot m_slot;
  };

  template<typename S, typename C>
  template<typename F>
  ServiceRequestSlotImplementation<S, C>::ServiceRequestSlotImplementation(
    F&& slot)
    : m_slot(std::forward<F>(slot)) {}

  template<typename S, typename C>
  void ServiceRequestSlotImplementation<S, C>::invoke(
      int id, Ref<ServiceProtocolClient> client,
      const typename Request::Parameters& parameters) const {
    try {
      for(auto& pre_hook : m_pre_hooks) {
        pre_hook(*client.get());
      }
      auto token =
        RequestToken<ServiceProtocolClient, Service>(Ref(client), id);
      std::apply([&] (const auto&... args) {
        m_slot(token, args...);
      }, boost::pfr::structure_tie(parameters));
    } catch(const std::exception& e) {
      client->send(Response(
        id, client->clone_exception(ServiceRequestException(e.what()))));
    }
  }

  template<typename S, typename C>
  void ServiceRequestSlotImplementation<S, C>::add_pre_hook(
      const PreHook& hook) {
    m_pre_hooks.push_back(hook);
  }

  template<typename T>
  struct ResponseResult {
    using Return = T;

    Return m_result;
  };

  template<>
  struct ResponseResult<void> {
    using Return = void;
  };
}

  /** Base class for a Request or Response Message. */
  template<typename C>
  class ServiceMessage : public Message<C> {
    public:

      /** Returns this Message's request id. */
      virtual int get_id() const = 0;

      /** Returns <code>true</code> iff this is a Response Message. */
      virtual bool is_response() const = 0;

      /**
       * Sets the result of this Request/Response.
       * @param eval The Eval to receive the result of this Request/Response.
       */
      virtual void set_eval(BaseEval& eval) const;
  };

  /**
   * Represents a request and response message.
   * @tparam R The type of value returned by the response.
   * @tparam P The Record representing the request's parameters.
   */
  template<typename R, typename P>
  class Service {
    public:

      /** The type returned by the Response. */
      using Return = R;

      /** The Record representing the request's parameters. */
      using Parameters = P;

      /**
       * Adds a slot to be associated with a Service Request.
       * @tparam C The type of ServiceProtocolClient receiving the Request.
       * @param service_slots The ServiceSlots to add the slot to.
       * @param slot The slot handling the Request.
       */
      template<typename C>
      static void add_request_slot(Out<ServiceSlots<C>> service_slots,
        const Details::request_slot_t<C, Service>& slot);

      /**
       * Adds a slot to be associated with a Service Request.
       * @tparam C The type of ServiceProtocolClient receiving the Request.
       * @param service_slots The ServiceSlots to add the slot to.
       * @param slot The slot handling the Request.
       */
      template<typename C>
      static void add_slot(Out<ServiceSlots<C>> service_slots,
        const Details::slot_t<C, Service>& slot);

      /**
       * Represents a request for a Service.
       * @tparam C The type of ServiceProtocolClient this Request is used with.
       */
      template<typename C>
      class Request : public ServiceMessage<C> {
        public:

          /** The type of ServiceProtocolClient this Request is used with. */
          using ServiceProtocolClient = C;

          /** The type of slot called when a request is received. */
          using Slot = Details::ServiceRequestSlot<Request>;

          /** The type returned by the Response. */
          using Return = R;

          /** The Record representing the request's parameters. */
          using Parameters = P;

          /**
           * Constructs a Request.
           * @param id The id identifying this Request.
           * @param parameters The Record containing the Parameters to send.
           */
          Request(int id, Parameters parameters);

          int get_id() const override;
          bool is_response() const override;
          void emit(BaseServiceSlot<ServiceProtocolClient>* slot,
            Ref<ServiceProtocolClient> protocol) const override;

        private:
          friend struct Beam::DataShuttle;
          int m_id;
          Parameters m_parameters;

          Request() = default;
          template<IsShuttle S>
          void shuttle(S& shuttle, unsigned int version);
      };

      /**
       * Represents the response to a Service Request.
       * @tparam C The type of ServiceProtocolClient this Request is used with.
       */
      template<typename C>
      class Response : public ServiceMessage<C>,
          private Details::ResponseResult<R> {
        public:

          /** The type of ServiceProtocolClient this Request is used with. */
          using ServiceProtocolClient = C;

          /**
           * Constructs a Response.
           * @param id The id of the request being responded to.
           * @param result The result of the Service Request.
           */
          template<typename Q>
          Response(int id, Q&& result) requires(
            !std::same_as<Q, R> || !std::same_as<R, void>);

          /**
           * Constructs a Response to a void Service.
           * @param id The id of the request being responded to.
           */
          explicit Response(int id);

          /**
           * Constructs a Response that failed with an exception.
           * @param id The id of the request being responded to.
           * @param e The ServiceRequestException that caused the Request to
           *          fail.
           */
          Response(int id, std::unique_ptr<ServiceRequestException> e);

          int get_id() const override;
          bool is_response() const override;
          void set_eval(BaseEval& eval) const override;
          void emit(BaseServiceSlot<ServiceProtocolClient>* slot,
            Ref<ServiceProtocolClient> protocol) const override;

        private:
          friend struct Beam::DataShuttle;
          int m_id;
          std::unique_ptr<ServiceRequestException> m_exception;

          Response() = default;
          template<IsSender S>
          void send(S& sender, unsigned int version) const;
          template<IsReceiver S>
          void receive(S& receiver, unsigned int version);
      };
  };

  template<typename C>
  void ServiceMessage<C>::set_eval(BaseEval& eval) const {}

  template<typename R, typename P>
  template<typename C>
  void Service<R, P>::add_request_slot(Out<ServiceSlots<C>> service_slots,
      const Details::request_slot_t<C, Service>& slot) {
    service_slots->add(std::make_unique<
      Details::ServiceRequestSlotImplementation<Service, C>>(slot));
  }

  template<typename R, typename P>
  template<typename C>
  void Service<R, P>::add_slot(Out<ServiceSlots<C>> service_slots,
      const Details::slot_t<C, Service>& slot) {
    add_request_slot(service_slots,
      [slot] <typename T, typename... Args> (T& request, Args&&... args) {
        try {
          if constexpr(std::same_as<Return, void>) {
            slot(request.get_client(), std::forward<Args>(args)...);
            request.set();
          } else {
            request.set(
              slot(request.get_client(), std::forward<Args>(args)...));
          }
        } catch(const ServiceRequestException& e) {
          request.set_exception(e);
        } catch(const std::exception& e) {
          request.set_exception(ServiceRequestException(e.what()));
        }
      });
  }

  template<typename R, typename P>
  template<typename C>
  Service<R, P>::Request<C>::Request(int id, P parameters)
    : m_id(id),
      m_parameters(std::move(parameters)) {}

  template<typename R, typename P>
  template<typename C>
  int Service<R, P>::Request<C>::get_id() const {
    return m_id;
  }

  template<typename R, typename P>
  template<typename C>
  bool Service<R, P>::Request<C>::is_response() const {
    return false;
  }

  template<typename R, typename P>
  template<typename C>
  void Service<R, P>::Request<C>::emit(
      BaseServiceSlot<ServiceProtocolClient>* slot,
      Ref<ServiceProtocolClient> protocol) const {
    static_cast<Slot*>(slot)->invoke(m_id, Ref(protocol), m_parameters);
  }

  template<typename R, typename P>
  template<typename C>
  template<IsShuttle S>
  void Service<R, P>::Request<C>::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("request_id", m_id);
    if(!std::is_empty_v<Parameters>) {
      shuttle.shuttle("parameters", m_parameters);
    }
  }

  template<typename R, typename P>
  template<typename C>
  template<typename Q>
  Service<R, P>::Response<C>::Response(int id, Q&& result) requires(
      !std::same_as<Q, R> || !std::same_as<R, void>)
    : Details::ResponseResult<R>(std::forward<Q>(result)),
      m_id(id) {}

  template<typename R, typename P>
  template<typename C>
  Service<R, P>::Response<C>::Response(int id)
      : m_id(id) {
    static_assert(
      std::same_as<R, void>, "Constructor only valid for void return type.");
  }

  template<typename R, typename P>
  template<typename C>
  Service<R, P>::Response<C>::Response(
    int id, std::unique_ptr<ServiceRequestException> e)
    : m_id(id),
      m_exception(std::move(e)) {}

  template<typename R, typename P>
  template<typename C>
  int Service<R, P>::Response<C>::get_id() const {
    return m_id;
  }

  template<typename R, typename P>
  template<typename C>
  bool Service<R, P>::Response<C>::is_response() const {
    return true;
  }

  template<typename R, typename P>
  template<typename C>
  void Service<R, P>::Response<C>::set_eval(BaseEval& eval) const {
    if(m_exception) {
      static_cast<Eval<R>&>(eval).set_exception(
        std::make_exception_ptr(*m_exception));
    } else if constexpr(std::same_as<Return, void>) {
      static_cast<Eval<R>&>(eval).set();
    } else {
      static_cast<Eval<R>&>(eval).set(std::move(this->m_result));
    }
  }

  template<typename R, typename P>
  template<typename C>
  void Service<R, P>::Response<C>::emit(
      BaseServiceSlot<ServiceProtocolClient>* slot,
      Ref<ServiceProtocolClient> protocol) const {
    assert(false);
  }

  template<typename R, typename P>
  template<typename C>
  template<IsSender S>
  void Service<R, P>::Response<C>::send(S& sender, unsigned int version) const {
    sender.send("request_id", m_id);
    auto is_exception = (m_exception != nullptr);
    sender.send("is_exception", is_exception);
    if(is_exception) {
      sender.send("result", m_exception);
    } else if constexpr(!std::same_as<Return, void>) {
      sender.send("result", this->m_result);
    }
  }

  template<typename R, typename P>
  template<typename C>
  template<IsReceiver S>
  void Service<R, P>::Response<C>::receive(S& receiver, unsigned int version) {
    receiver.receive("request_id", m_id);
    if(Beam::receive<bool>(receiver, "is_exception")) {
      receiver.receive("result", m_exception);
    } else if constexpr(!std::same_as<Return, void>) {
      receiver.receive("result", this->m_result);
    }
  }
}

#endif
