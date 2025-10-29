#ifndef BEAM_REQUEST_TOKEN_HPP
#define BEAM_REQUEST_TOKEN_HPP
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Services/ServiceRequestException.hpp"

namespace Beam {

  /**
   * Encapsulates a service request received from a ServiceProtocolClient.
   * @tparam C The type of ServiceProtocolClient the request was received from.
   * @tparam S The type of service requested.
   */
  template<typename C, typename S>
  class RequestToken {
    public:

      /** The type of ServiceProtocolClient the request was received from. */
      using ServiceProtocolClient = dereference_t<C>;

      /** The type of service requested. */
      using Service = S;

      /**
       * Constructs a RequestToken.
       * @param client The client making the request.
       * @param id The request's unique identifier.
       */
      RequestToken(Ref<ServiceProtocolClient> client, int id) noexcept;

      /** Returns the client that made the request. */
      ServiceProtocolClient& get_client() const;

      /** Returns the client's session. */
      typename ServiceProtocolClient::Session& get_session() const;

      /**
       * Sends the client a response to this request.
       * @param result The result to the send to the client.
       */
      template<typename Result>
      void set(Result&& result) const;

      /** Sends the client a result to a void request. */
      void set() const;

      /**
       * Sends the client an exception to the request.
       * @param e The service request exception.
       */
      void set_exception(const ServiceRequestException& e) const;

      /**
       * Sends the client an exception to the request.
       * @param e The service request exception.
       */
      void set_exception(const std::exception_ptr& e) const;

      /**
       * Sends the client an exception to the request.
       * @param e The service request exception.
       */
      template<typename E>
      void set_exception(const E& e) const;

    private:
      ServiceProtocolClient* m_client;
      int m_id;
  };

  template<typename C, typename S>
  RequestToken<C, S>::RequestToken(
    Ref<ServiceProtocolClient> client, int id) noexcept
    : m_client(client.get()),
      m_id(id) {}

  template<typename C, typename S>
  typename RequestToken<C, S>::ServiceProtocolClient&
      RequestToken<C, S>::get_client() const {
    return *m_client;
  }

  template<typename C, typename S>
  typename RequestToken<C, S>::ServiceProtocolClient::Session&
      RequestToken<C, S>::get_session() const {
    return get_client().get_session();
  }

  template<typename C, typename S>
  template<typename Result>
  void RequestToken<C, S>::set(Result&& result) const {
    get_client().send(typename Service::template Response<C>(
      m_id, std::forward<Result>(result)));
  }

  template<typename C, typename S>
  void RequestToken<C, S>::set() const {
    get_client().send(typename Service::template Response<C>(m_id));
  }

  template<typename C, typename S>
  void RequestToken<C, S>::set_exception(
      const ServiceRequestException& e) const {
    get_client().send(typename Service::template Response<C>(
      m_id, get_client().clone_exception(e)));
  }

  template<typename C, typename S>
  void RequestToken<C, S>::set_exception(const std::exception_ptr& e) const {
    try {
      std::rethrow_exception(e);
    } catch(const std::exception& ex) {
      set_exception(ServiceRequestException(ex.what()));
    }
  }

  template<typename C, typename S>
  template<typename E>
  void RequestToken<C, S>::set_exception(const E& e) const {
    set_exception(ServiceRequestException(e.what()));
  }
}

#endif
