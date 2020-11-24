#ifndef BEAM_SERVICE_HPP
#define BEAM_SERVICE_HPP
#include <functional>
#include <vector>
#include <boost/call_traits.hpp>
#include <boost/mpl/size.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/tuple/to_list.hpp>
#include "Beam/Routines/Async.hpp"
#include "Beam/Serialization/ShuttleNullType.hpp"
#include "Beam/Serialization/ShuttleRecord.hpp"
#include "Beam/Services/Message.hpp"
#include "Beam/Services/RequestToken.hpp"
#include "Beam/Services/ServiceRequestException.hpp"
#include "Beam/Services/ServiceSlot.hpp"
#include "Beam/Utilities/Preprocessor.hpp"

#define BEAM_DEFINE_SERVICE(Name, Uid, R, ...)                                 \
  namespace Details {                                                          \
    BEAM_DEFINE_RECORD(Name##Parameters, __VA_ARGS__)                          \
  }                                                                            \
                                                                               \
  using Name = ::Beam::Services::Service<R, Details::Name##Parameters>;

#define BEAM_APPLY_SERVICE(z, n, q) BEAM_DEFINE_SERVICE q
#define BEAM_GET_SERVICE_NAME(Name, ...) Name
#define BEAM_GET_SERVICE_UID(Name, Uid, ...) Uid

#define BEAM_REGISTER_SERVICE(z, n, q)                                         \
  slots->GetRegistry().template Register<BEAM_GET_SERVICE_NAME q ::Request<C>>(\
    BEAM_GET_SERVICE_UID q ".Request");                                        \
  slots->GetRegistry().template Register<BEAM_GET_SERVICE_NAME q               \
    ::Response<C>>(BEAM_GET_SERVICE_UID q ".Response");

#define BEAM_DEFINE_SERVICES_(Name, ServiceList)                               \
  BOOST_PP_LIST_FOR_EACH(BEAM_APPLY_SERVICE, BOOST_PP_EMPTY, ServiceList)      \
                                                                               \
  template<typename C>                                                         \
  void Register##Name(::Beam::Out< ::Beam::Services::ServiceSlots<C>> slots) { \
    BOOST_PP_LIST_FOR_EACH(BEAM_REGISTER_SERVICE, BOOST_PP_EMPTY, ServiceList) \
  }

#define BEAM_DEFINE_SERVICES(Name, ...)                                        \
  BEAM_DEFINE_SERVICES_(Name, BOOST_PP_TUPLE_TO_LIST(PP_NARG(__VA_ARGS__),     \
    (__VA_ARGS__)))

namespace Beam::Services {
namespace Details {
  template<typename T, typename TypeList =
    typename T::Service::Parameters::TypeList>
  struct GetSlotType {};

  template<typename T, typename TypeList =
    typename T::Service::Parameters::TypeList>
  struct GetSlotWrapperType {};

  template<typename T, typename TypeList =
    typename T::Service::Parameters::TypeList>
  struct InvokeSlot {};

  template<typename T, typename S,
    typename R = typename T::Service::Return,
    typename TypeList = typename T::Service::Parameters::TypeList>
  struct SlotWrapper {};

  #define PASS_PARAMETER(z, n, q)                                              \
    BOOST_PP_COMMA_IF(n) typename boost::call_traits<A##n>::param_type a##n

  #define GET_PARAMETER(z, n, q)                                               \
    BOOST_PP_COMMA_IF(n) parameters.template Get<n>()

  #define BOOST_PP_LOCAL_MACRO(n)                                              \
  template<typename T BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename A)>\
  struct GetSlotType<T, boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, A)>> {      \
    using type = std::function<void (T& BOOST_PP_COMMA_IF(n)                   \
      BOOST_PP_REPEAT(n, PASS_PARAMETER, BOOST_PP_EMPTY))>;                    \
  };                                                                           \
                                                                               \
  template<typename T BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename A)>\
  struct GetSlotWrapperType<T,                                                 \
      boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, A)>> {                        \
    using type = std::function<typename T::Service::Return (                   \
      typename T::ServiceProtocolClient& BOOST_PP_COMMA_IF(n)                  \
      BOOST_PP_REPEAT(n, PASS_PARAMETER, BOOST_PP_EMPTY))>;                    \
  };                                                                           \
                                                                               \
  template<typename T BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename A)>\
  struct InvokeSlot<T, boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, A)>> {       \
    void operator ()(const typename GetSlotType<T>::type& slot, T& token,      \
        const typename T::Service::Parameters& parameters) const {             \
      slot(token BOOST_PP_COMMA_IF(n) BOOST_PP_REPEAT(n, GET_PARAMETER,        \
        BOOST_PP_EMPTY));                                                      \
    }                                                                          \
  };                                                                           \
                                                                               \
  template<typename T, typename S                                              \
    BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename A)>                  \
  struct SlotWrapper<T, S, void,                                               \
      boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, A)>> {                        \
    typename GetSlotWrapperType<T>::type m_slot;                               \
                                                                               \
    SlotWrapper(typename GetSlotWrapperType<T>::type slot)                     \
      : m_slot(slot) {}                                                        \
                                                                               \
    void operator ()(T& request BOOST_PP_COMMA_IF(n)                           \
        BOOST_PP_REPEAT(n, PASS_PARAMETER, BOOST_PP_EMPTY)) const {            \
      try {                                                                    \
        m_slot(request.GetClient() BOOST_PP_COMMA_IF(n)                        \
          BOOST_PP_ENUM_PARAMS(n, a));                                         \
        request.SetResult();                                                   \
      } catch(const ServiceRequestException& e) {                              \
        request.SetException(e);                                               \
      } catch(const std::exception& e) {                                       \
        request.SetException(ServiceRequestException(e.what()));               \
      }                                                                        \
    }                                                                          \
  };                                                                           \
                                                                               \
  template<typename T, typename S, typename R                                  \
    BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename A)>                  \
  struct SlotWrapper<T, S, R, boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, A)>> {\
    typename GetSlotWrapperType<T>::type m_slot;                               \
                                                                               \
    SlotWrapper(typename GetSlotWrapperType<T>::type slot)                     \
      : m_slot(slot) {}                                                        \
                                                                               \
    void operator ()(T& request BOOST_PP_COMMA_IF(n)                           \
        BOOST_PP_REPEAT(n, PASS_PARAMETER, BOOST_PP_EMPTY)) const {            \
      try {                                                                    \
        request.SetResult(m_slot(request.GetClient() BOOST_PP_COMMA_IF(n)      \
          BOOST_PP_ENUM_PARAMS(n, a)));                                        \
      } catch(const ServiceRequestException& e) {                              \
        request.SetException(e);                                               \
      } catch(const std::exception& e) {                                       \
        request.SetException(ServiceRequestException(e.what()));               \
      }                                                                        \
    }                                                                          \
  };

  #define BOOST_PP_LOCAL_LIMITS (0, BEAM_SERVICE_PARAMETERS)
  #include BOOST_PP_LOCAL_ITERATE()
  #undef GET_PARAMETER
  #undef PASS_PARAMETER

  template<typename R>
  class ServiceRequestSlot : public ServiceSlot<R> {
    public:
      using Request = R;
      using PreHook = typename ServiceSlot<Request>::PreHook;

      virtual void Invoke(int requestId,
        Ref<typename Request::ServiceProtocolClient> protocol,
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
      using Slot = typename GetSlotType<RequestToken<C, Service>>::type;
      using PreHook = typename ServiceRequestSlot<
        typename S::template Request<C>>::PreHook;

      template<typename L>
      ServiceRequestSlotImplementation(L&& slot);

      void Invoke(int requestId, Ref<ServiceProtocolClient> protocol,
        const typename Request::Parameters& parameters) const override;

      void AddPreHook(const PreHook& hook) override;

    private:
      std::vector<PreHook> m_preHooks;
      Slot m_slot;
  };

  template<typename S, typename C>
  template<typename L>
  ServiceRequestSlotImplementation<S, C>::ServiceRequestSlotImplementation(
    L&& slot)
    : m_slot(std::forward<L>(slot)) {}

  template<typename S, typename C>
  void ServiceRequestSlotImplementation<S, C>::Invoke(int requestId,
      Ref<ServiceProtocolClient> protocol,
      const typename Request::Parameters& parameters) const {
    try {
      for(auto& preHook : m_preHooks) {
        preHook(*protocol.Get());
      }
      auto token = RequestToken<ServiceProtocolClient, Service>(Ref(protocol),
        requestId);
      InvokeSlot<RequestToken<ServiceProtocolClient, Service>>()(m_slot, token,
        parameters);
    } catch(const ServiceRequestException& e) {
      protocol->Send(Response(requestId, protocol->CloneException(e)));
    } catch(const std::exception& e) {
      protocol->Send(Response(requestId, protocol->CloneException(
        ServiceRequestException(e.what()))));
    }
  }

  template<typename S, typename C>
  void ServiceRequestSlotImplementation<S, C>::AddPreHook(const PreHook& hook) {
    m_preHooks.push_back(hook);
  }
}

  /** Base class for a Request or Response Message. */
  template<typename C>
  class ServiceMessage : public Message<C> {
    public:

      /** Returns this Message's request id. */
      virtual int GetRequestId() const = 0;

      /** Returns <code>true</code> iff this is a Response Message. */
      virtual bool IsResponseMessage() const = 0;

      /**
       * Sets the result of this Request/Response.
       * @param eval The Eval to receive the result of this Request/Response.
       */
      virtual void SetEval(Routines::BaseEval& eval) const;
  };

  /**
   * Represents a request and response message.
   * @param <R> The type of value returned by the response.
   * @param <P> The Record representing the request's parameters.
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
       * @param <C> The type of ServiceProtocolClient receiving the Request.
       * @param slot The slot handling the Request.
       */
      template<typename C>
      static void AddRequestSlot(Out<ServiceSlots<C>> serviceSlots,
        const typename Details::GetSlotType<RequestToken<C, Service>>::type&
        slot);

      /**
       * Adds a slot to be associated with a Service Request.
       * @param <C> The type of ServiceProtocolClient receiving the Request.
       * @param slot The slot handling the Request.
       */
      template<typename C>
      static void AddSlot(Out<ServiceSlots<C>> serviceSlots,
        const typename Details::GetSlotWrapperType<
        RequestToken<C, Service>>::type& slot);

      /**
       * Represents a request for a Service.
       * @param <C> The type of ServiceProtocolClient this Request is used with.
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
           * @param requestId The id identifying this Request.
           * @param parameters The Record containing the Parameters to send.
           */
          Request(int requestId, const Parameters& parameters);

          int GetRequestId() const override;

          bool IsResponseMessage() const override;

          void EmitSignal(BaseServiceSlot<ServiceProtocolClient>* slot,
            Ref<ServiceProtocolClient> protocol) const override;

        private:
          friend struct Serialization::DataShuttle;
          int m_requestId;
          Parameters m_parameters;

          Request() = default;
          template<typename Shuttler>
          void Shuttle(Shuttler& shuttle, unsigned int version);
      };

      /**
       * Represents the response to a Service Request.
       * @param <C> The type of ServiceProtocolClient this Request is used with.
       */
      template<typename C>
      class Response : public ServiceMessage<C> {
        public:

          /** The type of ServiceProtocolClient this Request is used with. */
          using ServiceProtocolClient = C;

          /**
           * Constructs a Response.
           * @param requestId The id of the request being responded to.
           * @param result The result of the Service Request.
           */
          template<typename Q>
          Response(int requestId, Q&& result, std::enable_if_t<
            !std::is_same<Q, R>::value || !std::is_same<R, void>::value>* = 0);

          /**
           * Constructs a Response to a void Service.
           * @param requestId The id of the request being responded to.
           */
          Response(int requestId);

          /**
           * Constructs a Response that failed with an exception.
           * @param requestId The id of the request being responded to.
           * @param e The ServiceRequestException that caused the Request to
           *          fail.
           */
          Response(int requestId, std::unique_ptr<ServiceRequestException> e);

          int GetRequestId() const override;

          bool IsResponseMessage() const override;

          void SetEval(Routines::BaseEval& eval) const override;

          void EmitSignal(BaseServiceSlot<ServiceProtocolClient>* slot,
            Ref<ServiceProtocolClient> protocol) const override;

        private:
          friend struct Serialization::DataShuttle;
          int m_requestId;
          typename StorageType<R>::type m_result;
          std::unique_ptr<ServiceRequestException> m_exception;

          Response() = default;
          template<typename Shuttler>
          void Send(Shuttler& shuttle, unsigned int version) const;
          template<typename Shuttler>
          void Receive(Shuttler& shuttle, unsigned int version);
      };
  };

  template<typename C>
  void ServiceMessage<C>::SetEval(Routines::BaseEval& eval) const {}

  template<typename R, typename P>
  template<typename C>
  void Service<R, P>::AddRequestSlot(Out<ServiceSlots<C>> serviceSlots,
      const typename Details::GetSlotType<
      RequestToken<C, Service>>::type& slot) {
    auto serviceSlot = std::unique_ptr<ServiceSlot<Request<C>>>(
      std::make_unique<Details::ServiceRequestSlotImplementation<Service, C>>(
      slot));
    serviceSlots->Add(std::move(serviceSlot));
  }

  template<typename R, typename P>
  template<typename C>
  void Service<R, P>::AddSlot(Out<ServiceSlots<C>> serviceSlots,
      const typename Details::GetSlotWrapperType<
      RequestToken<C, Service>>::type& slot) {
    auto slotWrapper = Details::SlotWrapper<RequestToken<C, Service>,
      typename Details::GetSlotWrapperType<RequestToken<C, Service>>::type>(
      slot);
    auto serviceSlot = std::unique_ptr<ServiceSlot<Request<C>>>(
      std::make_unique<Details::ServiceRequestSlotImplementation<Service, C>>(
      std::move(slotWrapper)));
    serviceSlots->Add(std::move(serviceSlot));
  }

  template<typename R, typename P>
  template<typename C>
  Service<R, P>::Request<C>::Request(int requestId, const P& parameters)
    : m_requestId(requestId),
      m_parameters(parameters) {}

  template<typename R, typename P>
  template<typename C>
  int Service<R, P>::Request<C>::GetRequestId() const {
    return m_requestId;
  }

  template<typename R, typename P>
  template<typename C>
  bool Service<R, P>::Request<C>::IsResponseMessage() const {
    return false;
  }

  template<typename R, typename P>
  template<typename C>
  void Service<R, P>::Request<C>::EmitSignal(
      BaseServiceSlot<ServiceProtocolClient>* slot,
      Ref<ServiceProtocolClient> protocol) const {
    static_cast<Slot*>(slot)->Invoke(m_requestId, Ref(protocol), m_parameters);
  }

  template<typename R, typename P>
  template<typename C>
  template<typename Shuttler>
  void Service<R, P>::Request<C>::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("request_id", m_requestId);
    if(boost::mpl::size<typename Parameters::TypeList>::value != 0) {
      shuttle.Shuttle("parameters", m_parameters);
    }
  }

  template<typename R, typename P>
  template<typename C>
  template<typename Q>
  Service<R, P>::Response<C>::Response(int requestId, Q&& result,
    std::enable_if_t<!std::is_same<Q, R>::value ||
      !std::is_same<R, void>::value>*)
    : m_requestId(requestId),
      m_result(std::forward<Q>(result)) {}

  template<typename R, typename P>
  template<typename C>
  Service<R, P>::Response<C>::Response(int requestId)
      : m_requestId(requestId) {
    static_assert(std::is_same<R, void>::value,
      "Constructor only valid for void return type.");
  }

  template<typename R, typename P>
  template<typename C>
  Service<R, P>::Response<C>::Response(int requestId,
    std::unique_ptr<ServiceRequestException> e)
    : m_requestId(requestId),
      m_exception(std::move(e)) {}

  template<typename R, typename P>
  template<typename C>
  int Service<R, P>::Response<C>::GetRequestId() const {
    return m_requestId;
  }

  template<typename R, typename P>
  template<typename C>
  bool Service<R, P>::Response<C>::IsResponseMessage() const {
    return true;
  }

  template<typename R, typename P>
  template<typename C>
  void Service<R, P>::Response<C>::SetEval(Routines::BaseEval& eval) const {
    if(m_exception != nullptr) {
      static_cast<Routines::Eval<R>&>(eval).SetException(
        std::make_exception_ptr(*m_exception));
    } else {
      static_cast<Routines::Eval<R>&>(eval).SetResult(std::move(m_result));
    }
  }

  template<typename R, typename P>
  template<typename C>
  void Service<R, P>::Response<C>::EmitSignal(
      BaseServiceSlot<ServiceProtocolClient>* slot,
      Ref<ServiceProtocolClient> protocol) const {
    assert(false);
  }

  template<typename R, typename P>
  template<typename C>
  template<typename Shuttler>
  void Service<R, P>::Response<C>::Send(Shuttler& shuttle,
      unsigned int version) const {
    shuttle.Shuttle("request_id", m_requestId);
    auto isException = (m_exception != nullptr);
    shuttle.Shuttle("is_exception", isException);
    if(isException) {
      shuttle.Shuttle("result", m_exception);
    } else {
      shuttle.Shuttle("result", m_result);
    }
  }

  template<typename R, typename P>
  template<typename C>
  template<typename Shuttler>
  void Service<R, P>::Response<C>::Receive(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("request_id", m_requestId);
    auto isException = bool();
    shuttle.Shuttle("is_exception", isException);
    if(isException) {
      shuttle.Shuttle("result", m_exception);
    } else {
      shuttle.Shuttle("result", m_result);
    }
  }
}

#endif
