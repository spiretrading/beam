#ifndef BEAM_REQUESTTOKEN_HPP
#define BEAM_REQUESTTOKEN_HPP
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Services/ServiceRequestException.hpp"

namespace Beam {
namespace Services {

  /*! \class RequestToken
      \brief Encapsulates a service request received from a
             ServiceProtocolClient.
      \tparam ServiceProtocolClientType The type of ServiceProtocolClient the
              request was received from.
      \tparam ServiceType The type of service requested.
   */
  template<typename ServiceProtocolClientType, typename ServiceType>
  class RequestToken {
    public:

      //! The type of ServiceProtocolClient the request was received from.
      typedef typename TryDereferenceType<ServiceProtocolClientType>::type
        ServiceProtocolClient;

      //! The type of service requested.
      typedef ServiceType Service;

      //! Constructs a RequestToken.
      /*!
        \param client The client making the request.
        \param requestId The request's unique identifier.
      */
      RequestToken(Ref<ServiceProtocolClient> client, int requestId);

      //! Returns the client that made the request.
      ServiceProtocolClient& GetClient() const;

      //! Returns the client's session.
      typename ServiceProtocolClient::Session& GetSession() const;

      //! Sends the client a response to this request.
      /*!
        \param result The result to the send to the client.
      */
      template<typename ResultForward>
      void SetResult(ResultForward&& result) const;

      //! Sends the client a result to a void request.
      void SetResult() const;

      //! Sends the client an exception to the request.
      /*!
        \param e The service request exception.
      */
      void SetException(const ServiceRequestException& e) const;

      //! Sends the client an exception to the request.
      /*!
        \param e The service request exception.
      */
      void SetException(const std::exception_ptr& e) const;

      //! Sends the client an exception to the request.
      /*!
        \param e The service request exception.
      */
      template<typename E>
      void SetException(const E& e) const;

    private:
      ServiceProtocolClient* m_client;
      int m_requestId;
  };

  template<typename ServiceProtocolClientType, typename ServiceType>
  RequestToken<ServiceProtocolClientType, ServiceType>::RequestToken(
      Ref<ServiceProtocolClient> client, int requestId)
      : m_client(client.Get()),
        m_requestId(requestId) {}

  template<typename ServiceProtocolClientType, typename ServiceType>
  typename RequestToken<ServiceProtocolClientType, ServiceType>::
      ServiceProtocolClient& RequestToken<ServiceProtocolClientType,
      ServiceType>::GetClient() const {
    return *m_client;
  }

  template<typename ServiceProtocolClientType, typename ServiceType>
  typename RequestToken<ServiceProtocolClientType, ServiceType>::
      ServiceProtocolClient::Session& RequestToken<ServiceProtocolClientType,
      ServiceType>::GetSession() const {
    return GetClient().GetSession();
  }

  template<typename ServiceProtocolClientType, typename ServiceType>
  template<typename ResultForward>
  void RequestToken<ServiceProtocolClientType, ServiceType>::SetResult(
      ResultForward&& result) const {
    GetClient().Send(typename Service::template Response<
      ServiceProtocolClientType>(m_requestId,
      std::forward<ResultForward>(result)));
  }

  template<typename ServiceProtocolClientType, typename ServiceType>
  void RequestToken<ServiceProtocolClientType, ServiceType>::SetResult() const{
    GetClient().Send(typename Service::template Response<
      ServiceProtocolClientType>(m_requestId));
  }

  template<typename ServiceProtocolClientType, typename ServiceType>
  void RequestToken<ServiceProtocolClientType, ServiceType>::SetException(
      const ServiceRequestException& e) const {
    GetClient().Send(typename Service::template Response<
      ServiceProtocolClientType>(m_requestId, GetClient().CloneException(e)));
  }

  template<typename ServiceProtocolClientType, typename ServiceType>
  void RequestToken<ServiceProtocolClientType, ServiceType>::SetException(
      const std::exception_ptr& e) const {
    try {
      std::rethrow_exception(e);
    } catch(const std::exception& ex) {
      SetException(ServiceRequestException(ex.what()));
    }
  }

  template<typename ServiceProtocolClientType, typename ServiceType>
  template<typename E>
  void RequestToken<ServiceProtocolClientType, ServiceType>::SetException(
      const E& e) const {
    SetException(ServiceRequestException(e.what()));
  }
}
}

#endif
