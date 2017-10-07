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
#include "Beam/Pointers/UniquePtr.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Serialization/ShuttleNullType.hpp"
#include "Beam/Serialization/ShuttleRecord.hpp"
#include "Beam/Services/Message.hpp"
#include "Beam/Services/RequestToken.hpp"
#include "Beam/Services/ServiceRequestException.hpp"
#include "Beam/Services/ServiceSlot.hpp"
#include "Beam/Utilities/Preprocessor.hpp"

#define BEAM_DEFINE_SERVICE(Name, Uid, ReturnType, ...)                        \
  namespace Details {                                                          \
    BEAM_DEFINE_RECORD(Name##Parameters, __VA_ARGS__)                          \
  }                                                                            \
                                                                               \
  typedef ::Beam::Services::Service<ReturnType, Details::Name##Parameters>     \
    Name;

#define BEAM_APPLY_SERVICE(z, n, q) BEAM_DEFINE_SERVICE q
#define BEAM_GET_SERVICE_NAME(Name, ...) Name
#define BEAM_GET_SERVICE_UID(Name, Uid, ...) Uid

#define BEAM_REGISTER_SERVICE(z, n, q)                                         \
  slots->GetRegistry().template Register<BEAM_GET_SERVICE_NAME q ::Request<    \
    ServiceProtocolClientType>>(BEAM_GET_SERVICE_UID q ".Request");            \
  slots->GetRegistry().template Register<BEAM_GET_SERVICE_NAME q ::Response<   \
    ServiceProtocolClientType>>(BEAM_GET_SERVICE_UID q ".Response");

#define BEAM_DEFINE_SERVICES_(Name, ServiceList)                               \
  BOOST_PP_LIST_FOR_EACH(BEAM_APPLY_SERVICE, BOOST_PP_EMPTY, ServiceList)      \
                                                                               \
  template<typename ServiceProtocolClientType>                                 \
  void Register##Name(::Beam::Out< ::Beam::Services::ServiceSlots<             \
      ServiceProtocolClientType>> slots) {                                     \
    BOOST_PP_LIST_FOR_EACH(BEAM_REGISTER_SERVICE, BOOST_PP_EMPTY, ServiceList) \
  }

#define BEAM_DEFINE_SERVICES(Name, ...)                                        \
  BEAM_DEFINE_SERVICES_(Name, BOOST_PP_TUPLE_TO_LIST(PP_NARG(__VA_ARGS__),     \
    (__VA_ARGS__)))

namespace Beam {
namespace Services {
namespace Details {
  template<typename RequestTokenType, typename TypeList =
    typename RequestTokenType::Service::Parameters::TypeList>
  struct GetSlotType {};

  template<typename RequestTokenType, typename TypeList =
    typename RequestTokenType::Service::Parameters::TypeList>
  struct GetSlotWrapperType {};

  template<typename RequestTokenType, typename TypeList =
    typename RequestTokenType::Service::Parameters::TypeList>
  struct InvokeSlot {};

  template<typename RequestTokenType, typename SlotType,
    typename ReturnType = typename RequestTokenType::Service::Return,
    typename TypeList =
    typename RequestTokenType::Service::Parameters::TypeList>
  struct SlotWrapper {};

  #define PASS_PARAMETER(z, n, q)                                              \
    BOOST_PP_COMMA_IF(n) typename boost::call_traits<A##n>::param_type a##n

  #define GET_PARAMETER(z, n, q)                                               \
    BOOST_PP_COMMA_IF(n) parameters.template Get<n>()

  #define BOOST_PP_LOCAL_MACRO(n)                                              \
  template<typename RequestTokenType BOOST_PP_COMMA_IF(n)                      \
    BOOST_PP_ENUM_PARAMS(n, typename A)>                                       \
  struct GetSlotType<RequestTokenType,                                         \
      boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, A)>> {                        \
    typedef std::function<void (RequestTokenType& BOOST_PP_COMMA_IF(n)         \
      BOOST_PP_REPEAT(n, PASS_PARAMETER, BOOST_PP_EMPTY))> type;               \
  };                                                                           \
                                                                               \
  template<typename RequestTokenType BOOST_PP_COMMA_IF(n)                      \
    BOOST_PP_ENUM_PARAMS(n, typename A)>                                       \
  struct GetSlotWrapperType<RequestTokenType,                                  \
      boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, A)>> {                        \
    typedef std::function<typename RequestTokenType::Service::Return (         \
      typename RequestTokenType::ServiceProtocolClient& BOOST_PP_COMMA_IF(n)   \
      BOOST_PP_REPEAT(n, PASS_PARAMETER, BOOST_PP_EMPTY))> type;               \
  };                                                                           \
                                                                               \
  template<typename RequestTokenType BOOST_PP_COMMA_IF(n)                      \
    BOOST_PP_ENUM_PARAMS(n, typename A)>                                       \
  struct InvokeSlot<RequestTokenType, boost::mpl::vector<                      \
      BOOST_PP_ENUM_PARAMS(n, A)>> {                                           \
    void operator ()(const typename GetSlotType<RequestTokenType>::type& slot, \
        RequestTokenType& token,                                               \
        const typename RequestTokenType::Service::Parameters& parameters)      \
        const {                                                                \
      slot(token BOOST_PP_COMMA_IF(n) BOOST_PP_REPEAT(n, GET_PARAMETER,        \
        BOOST_PP_EMPTY));                                                      \
    }                                                                          \
  };                                                                           \
                                                                               \
  template<typename RequestTokenType, typename SlotType                        \
    BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename A)>                  \
  struct SlotWrapper<RequestTokenType, SlotType, void,                         \
      boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, A)>> {                        \
    typename GetSlotWrapperType<RequestTokenType>::type m_slot;                \
                                                                               \
    SlotWrapper(typename GetSlotWrapperType<RequestTokenType>::type slot)      \
        : m_slot(slot) {}                                                      \
                                                                               \
    void operator ()(RequestTokenType& request BOOST_PP_COMMA_IF(n)            \
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
  template<typename RequestTokenType, typename SlotType, typename ReturnType   \
    BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename A)>                  \
  struct SlotWrapper<RequestTokenType, SlotType, ReturnType,                   \
      boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, A)>> {                        \
    typename GetSlotWrapperType<RequestTokenType>::type m_slot;                \
                                                                               \
    SlotWrapper(typename GetSlotWrapperType<RequestTokenType>::type slot)      \
        : m_slot(slot) {}                                                      \
                                                                               \
    void operator ()(RequestTokenType& request BOOST_PP_COMMA_IF(n)            \
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

  template<typename RequestType>
  class ServiceRequestSlot : public ServiceSlot<RequestType> {
    public:
      typedef RequestType Request;
      typedef typename ServiceSlot<RequestType>::PreHook PreHook;

      virtual void Invoke(int requestId,
        RefType<typename Request::ServiceProtocolClient> protocol,
        const typename Request::Parameters& parameters) const = 0;
  };

  template<typename ServiceType, typename ServiceProtocolClientType>
  class ServiceRequestSlotImplementation : public ServiceRequestSlot<
      typename ServiceType::template Request<ServiceProtocolClientType> > {
    public:
      typedef ServiceType Service;
      typedef ServiceProtocolClientType ServiceProtocolClient;
      typedef typename Service::template Request<ServiceProtocolClient> Request;
      typedef typename Service::template Response<ServiceProtocolClient>
        Response;
      typedef typename GetSlotType<RequestToken<ServiceProtocolClientType,
        Service>>::type Slot;
      typedef typename ServiceRequestSlot<
        typename ServiceType::template Request<
        ServiceProtocolClientType> >::PreHook PreHook;

      template<typename SlotForward>
      ServiceRequestSlotImplementation(SlotForward&& slot);

      virtual void Invoke(int requestId, RefType<ServiceProtocolClient>
        protocol, const typename Request::Parameters& parameters) const;

      virtual void AddPreHook(const PreHook& hook);

    private:
      std::vector<PreHook> m_preHooks;
      Slot m_slot;
  };

  template<typename ServiceType, typename ServiceProtocolClientType>
  template<typename SlotForward>
  ServiceRequestSlotImplementation<ServiceType, ServiceProtocolClientType>::
      ServiceRequestSlotImplementation(SlotForward&& slot)
      : m_slot(std::forward<SlotForward>(slot)) {}

  template<typename ServiceType, typename ServiceProtocolClientType>
  void ServiceRequestSlotImplementation<ServiceType,
      ServiceProtocolClientType>::Invoke(int requestId,
      RefType<ServiceProtocolClient> protocol,
      const typename Request::Parameters& parameters) const {
    try {
      for(const PreHook& preHook : m_preHooks) {
        preHook(*protocol.Get());
      }
    } catch(const ServiceRequestException& e) {
      protocol->Send(Response(requestId, protocol->CloneException(e)));
      return;
    } catch(const std::exception& e) {
      protocol->Send(Response(requestId, protocol->CloneException(
        ServiceRequestException(e.what()))));
      return;
    }
    RequestToken<ServiceProtocolClientType, Service> token(Ref(protocol),
      requestId);
    InvokeSlot<RequestToken<ServiceProtocolClientType, Service>>()(m_slot,
      token, parameters);
  }

  template<typename ServiceType, typename ServiceProtocolClientType>
  void ServiceRequestSlotImplementation<ServiceType,
      ServiceProtocolClientType>::AddPreHook(const PreHook& hook) {
    m_preHooks.push_back(hook);
  }
}

  /*! \class ServiceMessage
      \brief Base class for a Request or Response Message.
   */
  template<typename ServiceProtocolClientType>
  class ServiceMessage : public Message<ServiceProtocolClientType> {
    public:

      //! Returns this Message's request id.
      virtual int GetRequestId() const = 0;

      //! Returns <code>true</code> iff this is a Response Message.
      virtual bool IsResponseMessage() const = 0;

      //! Sets the result of this Request/Response.
      /*!
        \param eval The Eval to receive the result of this Request/Response.
      */
      virtual void SetEval(Routines::BaseEval& eval) const;
  };

  /*! \class Service
      \brief Represents a request and response message.
      \tparam ReturnType The type of value returned by the response.
      \tparam ParametersType The Record representing the request's parameters.
   */
  template<typename ReturnType, typename ParametersType>
  class Service {
    public:

      //! The type returned by the Response.
      typedef ReturnType Return;

      //! The Record representing the request's parameters.
      typedef ParametersType Parameters;

      //! Adds a slot to be associated with a Service Request.
      /*!
        \tparam ServiceProtocolClientType The type of ServiceProtocolClient
                receiving the Request.
        \param slot The slot handling the Request.
      */
      template<typename ServiceProtocolClientType>
      static void AddRequestSlot(Out<ServiceSlots<ServiceProtocolClientType>>
        serviceSlots, const typename Details::GetSlotType<
        RequestToken<ServiceProtocolClientType, Service>>::type& slot);

      //! Adds a slot to be associated with a Service Request.
      /*!
        \tparam ServiceProtocolClientType The type of ServiceProtocolClient
                receiving the Request.
        \param slot The slot handling the Request.
      */
      template<typename ServiceProtocolClientType>
      static void AddSlot(Out<ServiceSlots<ServiceProtocolClientType>>
        serviceSlots, const typename Details::GetSlotWrapperType<
        RequestToken<ServiceProtocolClientType, Service>>::type& slot);

      /*! \class Request
          \brief Represents a request for a Service.
          \tparam ServiceProtocolClientType The type of ServiceProtocolClient
                  this Request is used with.
       */
      template<typename ServiceProtocolClientType>
      class Request : public ServiceMessage<ServiceProtocolClientType> {
        public:

          //! The type of ServiceProtocolClient this Request is used with.
          typedef ServiceProtocolClientType ServiceProtocolClient;

          //! The type of slot called when a request is received.
          typedef Details::ServiceRequestSlot<Request> Slot;

          //! The type returned by the Response.
          typedef ReturnType Return;

          //! The Record representing the request's parameters.
          typedef ParametersType Parameters;

          //! Constructs a Request.
          /*!
            \param requestId The id identifying this Request.
            \param parameters The Record containing the Parameters to send.
          */
          Request(int requestId, const Parameters& parameters);

          virtual int GetRequestId() const;

          virtual bool IsResponseMessage() const;

          virtual void EmitSignal(BaseServiceSlot<ServiceProtocolClient>* slot,
            RefType<ServiceProtocolClient> protocol) const;

        private:
          friend struct Serialization::DataShuttle;
          int m_requestId;
          Parameters m_parameters;

          Request();
          template<typename Shuttler>
          void Shuttle(Shuttler& shuttle, unsigned int version);
      };

      /*! \class Response
          \brief Represents the response to a Service Request.
          \tparam ServiceProtocolClientType The type of ServiceProtocolClient
                  this Request is used with.
       */
      template<typename ServiceProtocolClientType>
      class Response : public ServiceMessage<ServiceProtocolClientType> {
        public:

          //! The type of ServiceProtocolClient this Request is used with.
          typedef ServiceProtocolClientType ServiceProtocolClient;

          //! Constructs a Response.
          /*!
            \param requestId The id of the request being responded to.
            \param result The result of the Service Request.
          */
          template<typename Q>
          Response(int requestId, Q&& result, typename std::enable_if<
            !std::is_same<Q, ReturnType>::value ||
            !std::is_same<ReturnType, void>::value>::type* = 0);

          //! Constructs a Response to a void Service.
          /*!
            \param requestId The id of the request being responded to.
          */
          Response(int requestId);

          //! Constructs a Response that failed with an exception.
          /*!
            \param requestId The id of the request being responded to.
            \param e The ServiceRequestException that caused the Request to
                     fail.
          */
          Response(int requestId, std::unique_ptr<ServiceRequestException> e);

          virtual int GetRequestId() const;

          virtual bool IsResponseMessage() const;

          virtual void SetEval(Routines::BaseEval& eval) const;

          virtual void EmitSignal(BaseServiceSlot<ServiceProtocolClient>* slot,
            RefType<ServiceProtocolClient> protocol) const;

        private:
          friend struct Serialization::DataShuttle;
          int m_requestId;
          typename StorageType<ReturnType>::type m_result;
          std::unique_ptr<ServiceRequestException> m_exception;

          Response();
          template<typename Shuttler>
          void Send(Shuttler& shuttle, unsigned int version) const;
          template<typename Shuttler>
          void Receive(Shuttler& shuttle, unsigned int version);
      };
  };

  template<typename ServiceProtocolClientType>
  void ServiceMessage<ServiceProtocolClientType>::SetEval(
    Routines::BaseEval& eval) const {}

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  void Service<ReturnType, ParametersType>::AddRequestSlot(
      Out<ServiceSlots<ServiceProtocolClientType>> serviceSlots,
      const typename Details::GetSlotType<
      RequestToken<ServiceProtocolClientType, Service>>::type& slot) {
    std::unique_ptr<ServiceSlot<Request<ServiceProtocolClientType>>>
      serviceSlot = std::make_unique<
      Details::ServiceRequestSlotImplementation<Service,
      ServiceProtocolClientType>>(slot);
    serviceSlots->Add(std::move(serviceSlot));
  }

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  void Service<ReturnType, ParametersType>::AddSlot(
      Out<ServiceSlots<ServiceProtocolClientType>> serviceSlots,
      const typename Details::GetSlotWrapperType<
      RequestToken<ServiceProtocolClientType, Service>>::type& slot) {
    Details::SlotWrapper<RequestToken<ServiceProtocolClientType, Service>,
      typename Details::GetSlotWrapperType<
      RequestToken<ServiceProtocolClientType, Service>>::type> slotWrapper(
      slot);
    std::unique_ptr<ServiceSlot<Request<ServiceProtocolClientType>>>
      serviceSlot = std::make_unique<
      Details::ServiceRequestSlotImplementation<Service,
      ServiceProtocolClientType>>(std::move(slotWrapper));
    serviceSlots->Add(std::move(serviceSlot));
  }

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  Service<ReturnType, ParametersType>::Request<ServiceProtocolClientType>::
      Request(int requestId, const ParametersType& parameters)
    : m_requestId(requestId),
      m_parameters(parameters) {}

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  int Service<ReturnType, ParametersType>::Request<ServiceProtocolClientType>::
      GetRequestId() const {
    return m_requestId;
  }

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  bool Service<ReturnType, ParametersType>::Request<ServiceProtocolClientType>::
      IsResponseMessage() const {
    return false;
  }

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  void Service<ReturnType, ParametersType>::Request<ServiceProtocolClientType>::
      EmitSignal(BaseServiceSlot<ServiceProtocolClient>* slot,
      RefType<ServiceProtocolClient> protocol) const {
    static_cast<Slot*>(slot)->Invoke(m_requestId, Ref(protocol), m_parameters);
  }

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  Service<ReturnType, ParametersType>::Request<ServiceProtocolClientType>::
    Request() {}

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  template<typename Shuttler>
  void Service<ReturnType, ParametersType>::Request<ServiceProtocolClientType>::
      Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("request_id", m_requestId);
    if(boost::mpl::size<typename Parameters::TypeList>::value != 0) {
      shuttle.Shuttle("parameters", m_parameters);
    }
  }

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  template<typename Q>
  Service<ReturnType, ParametersType>::Response<ServiceProtocolClientType>::
      Response(int requestId, Q&& result,
      typename std::enable_if<!std::is_same<Q, ReturnType>::value ||
      !std::is_same<ReturnType, void>::value>::type*)
      : m_requestId(requestId),
        m_result(std::forward<Q>(result)) {}

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  Service<ReturnType, ParametersType>::Response<ServiceProtocolClientType>::
      Response(int requestId)
      : m_requestId(requestId) {
    static_assert(std::is_same<ReturnType, void>::value,
      "Constructor only valid for void return type.");
  }

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  Service<ReturnType, ParametersType>::Response<ServiceProtocolClientType>::
      Response(int requestId, std::unique_ptr<ServiceRequestException> e)
      : m_requestId(requestId),
        m_exception(std::move(e)) {}

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  int Service<ReturnType, ParametersType>::Response<ServiceProtocolClientType>::
      GetRequestId() const {
    return m_requestId;
  }

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  bool Service<ReturnType, ParametersType>::Response<
      ServiceProtocolClientType>::IsResponseMessage() const {
    return true;
  }

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  void Service<ReturnType, ParametersType>::Response<
      ServiceProtocolClientType>::SetEval(Routines::BaseEval& eval) const {
    if(m_exception != nullptr) {
      static_cast<Routines::Eval<ReturnType>&>(eval).SetException(
        std::make_exception_ptr(*m_exception));
    } else {
      static_cast<Routines::Eval<ReturnType>&>(eval).SetResult(
        std::move(m_result));
    }
  }

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  void Service<ReturnType, ParametersType>::Response<
      ServiceProtocolClientType>::EmitSignal(
      BaseServiceSlot<ServiceProtocolClient>* slot,
      RefType<ServiceProtocolClient> protocol) const {
    assert(false);
  }

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  Service<ReturnType, ParametersType>::Response<ServiceProtocolClientType>::
    Response() {}

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  template<typename Shuttler>
  void Service<ReturnType, ParametersType>::Response<
      ServiceProtocolClientType>::Send(Shuttler& shuttle,
      unsigned int version) const {
    shuttle.Shuttle("request_id", m_requestId);
    bool isException = (m_exception != nullptr);
    shuttle.Shuttle("is_exception", isException);
    if(isException) {
      shuttle.Shuttle("result", m_exception);
    } else {
      shuttle.Shuttle("result", m_result);
    }
  }

  template<typename ReturnType, typename ParametersType>
  template<typename ServiceProtocolClientType>
  template<typename Shuttler>
  void Service<ReturnType, ParametersType>::Response<
      ServiceProtocolClientType>::Receive(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("request_id", m_requestId);
    bool isException;
    shuttle.Shuttle("is_exception", isException);
    if(isException) {
      shuttle.Shuttle("result", m_exception);
    } else {
      shuttle.Shuttle("result", m_result);
    }
  }
}
}

#endif
