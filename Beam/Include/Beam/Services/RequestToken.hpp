#ifndef BEAM_REQUEST_TOKEN_HPP
#define BEAM_REQUEST_TOKEN_HPP
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Services/ServiceRequestException.hpp"

namespace Beam::Services {

  /**
   * Encapsulates a service request received from a ServiceProtocolClient.
   * @param <C> The type of ServiceProtocolClient the request was received from.
   * @param <S> The type of service requested.
   */
  template<typename C, typename S>
  class RequestToken {
    public:

      /** The type of ServiceProtocolClient the request was received from. */
      using ServiceProtocolClient = GetTryDereferenceType<C>;

      /** The type of service requested. */
      using Service = S;

      /**
       * Constructs a RequestToken.
       * @param client The client making the request.
       * @param requestId The request's unique identifier.
       */
      RequestToken(Ref<ServiceProtocolClient> client, int requestId);

      /** Returns the client that made the request. */
      ServiceProtocolClient& GetClient() const;

      /** Returns the client's session. */
      typename ServiceProtocolClient::Session& GetSession() const;

      /**
       * Sends the client a response to this request.
       * @param result The result to the send to the client.
       */
      template<typename Result>
      void SetResult(Result&& result) const;

      /** Sends the client a result to a void request. */
      void SetResult() const;

      /**
       * Sends the client an exception to the request.
       * \param e The service request exception.
       */
      void SetException(const ServiceRequestException& e) const;

      /**
       * Sends the client an exception to the request.
       * @param e The service request exception.
       */
      void SetException(const std::exception_ptr& e) const;

      /**
       * Sends the client an exception to the request.
       * @param e The service request exception.
       */
      template<typename E>
      void SetException(const E& e) const;

    private:
      ServiceProtocolClient* m_client;
      int m_requestId;
  };

  template<typename C, typename S>
  RequestToken<C, S>::RequestToken(Ref<ServiceProtocolClient> client,
    int requestId)
    : m_client(client.Get()),
      m_requestId(requestId) {}

  template<typename C, typename S>
  typename RequestToken<C, S>::ServiceProtocolClient&
      RequestToken<C, S>::GetClient() const {
    return *m_client;
  }

  template<typename C, typename S>
  typename RequestToken<C, S>::ServiceProtocolClient::Session&
      RequestToken<C, S>::GetSession() const {
    return GetClient().GetSession();
  }

  template<typename C, typename S>
  template<typename Result>
  void RequestToken<C, S>::SetResult(Result&& result) const {
    GetClient().Send(typename Service::template Response<C>(m_requestId,
      std::forward<Result>(result)));
  }

  template<typename C, typename S>
  void RequestToken<C, S>::SetResult() const {
    GetClient().Send(typename Service::template Response<C>(m_requestId));
  }

  template<typename C, typename S>
  void RequestToken<C, S>::SetException(
      const ServiceRequestException& e) const {
    GetClient().Send(typename Service::template Response<C>(
      m_requestId, GetClient().CloneException(e)));
  }

  template<typename C, typename S>
  void RequestToken<C, S>::SetException(const std::exception_ptr& e) const {
    try {
      std::rethrow_exception(e);
    } catch(const std::exception& ex) {
      SetException(ServiceRequestException(ex.what()));
    }
  }

  template<typename C, typename S>
  template<typename E>
  void RequestToken<C, S>::SetException(const E& e) const {
    SetException(ServiceRequestException(e.what()));
  }
}

#endif
