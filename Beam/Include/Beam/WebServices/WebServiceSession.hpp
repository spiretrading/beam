#ifndef AVALON_WEBSERVICESESSION_HPP
#define AVALON_WEBSERVICESESSION_HPP
#include <boost/mpl/size.hpp>
#include <boost/thread/mutex.hpp>
#include "Avalon/IO/Buffer.hpp"
#include "Avalon/IO/Channel.hpp"
#include "Avalon/IOTests/IOTests.hpp"
#include "Avalon/Services/ServiceProtocol.hpp"
#include "Avalon/WebServices/HttpServerRequest.hpp"
#include "Avalon/WebServices/HttpServerResponse.hpp"
#include "Avalon/WebServices/HttpSession.hpp"

namespace Avalon {
namespace WebServices {

  /*! \class WebServiceSession
      \brief Proxies HttpSessions into ServiceProtocolChannels.
   */
  class WebServiceSession : public HttpSession {
    public:

      //! Constructs a WebServiceSession.
      /*!
        \param serverConnection The MockServerConnection to proxy client
                                Connections towards.
        \param serializer Used to serialize outgoing Service Messages.
        \param deserializer Used to deserialize incoming Service Messages.
      */
      WebServiceSession(IO::Tests::MockServerConnection* serverConnection,
        Serialization::Serializer* serializer,
        Serialization::Deserializer* deserializer);

      virtual ~WebServiceSession();

      //! Frees all resources associated with this session.
      void Shutdown();

      //! Handles a service request.
      /*!
        \tparam Service The type of Service request to handle.
        \param request The HTTP request encapsulating the <i>Service</i>
                       request.
        \param response Where to send the response for the <i>Service</i>.
      */
      template<typename Service>
      void HandleRequest(HttpServerRequest* request,
        HttpServerResponse* response);

    private:
      boost::mutex m_mutex;
      boost::scoped_ptr<Serialization::Serializer> m_serializer;
      boost::scoped_ptr<Serialization::Deserializer> m_deserializer;
      boost::scoped_ptr<IO::Tests::MockClientChannel> m_channel;
      Services::ServiceProtocol m_serviceProtocol;

      template<typename Service>
      void OnResponse(HttpServerRequest* request, HttpServerResponse* response,
        Threading::Async<typename Service::ReturnType>* result);
  };

  template<typename Service>
  void WebServiceSession::HandleRequest(HttpServerRequest* request,
      HttpServerResponse* response) {
    typename Service::Parameters parameters;
    if(boost::mpl::size<
        typename Service::Parameters::TypeList>::value != 0) {
      m_deserializer->SetDataSource(request->GetBody().GetData(),
        request->GetBody().GetSize());
      try {
        m_deserializer->Shuttle(parameters);
      } catch(std::exception&) {
        response->SetStatus(HttpStatusCode::BAD_REQUEST);
        response->SendResponse();
        return;
      }
    }
    Threading::Async<typename Service::ReturnType>* result =
      new Threading::Async<typename Service::ReturnType>();
    typename Threading::Async<typename Service::ReturnType>::State state;
    result->SetFinishedSlot(boost::bind(&WebServiceSession::OnResponse<Service>,
      this, request, response, result), Store(state));
    m_serviceProtocol.SendRequest<Service>(parameters, Store(*result));

    // TODO: Keep track of pending requests, if the session dies with a pending
    // request the behavior is undefined.
  }

  template<typename Service>
  void WebServiceSession::OnResponse(HttpServerRequest* request,
      HttpServerResponse* response,
      Threading::Async<typename Service::ReturnType>* result) {
    boost::scoped_ptr<Threading::Async<typename Service::ReturnType> >
      scopedResult(result);
    response->SetStatus(HttpStatusCode::OK);
    try {
      typename Service::Response serviceResponse(0, result->Get());
      boost::lock_guard<boost::mutex> lock(m_mutex);
      m_serializer->Shuttle(serviceResponse);
      response->SetBody(m_serializer->GetSerializedData());
    } catch(Services::ServiceRequestException& e) {
      typename Service::Response serviceResponse(0, e);
      boost::lock_guard<boost::mutex> lock(m_mutex);
      m_serializer->Shuttle(serviceResponse);
      response->SetBody(m_serializer->GetSerializedData());
    } catch(std::exception& e) {
      typename Service::Response serviceResponse(0,
        Services::ServiceRequestException(e.what()));
      boost::lock_guard<boost::mutex> lock(m_mutex);
      m_serializer->Shuttle(serviceResponse);
      response->SetBody(m_serializer->GetSerializedData());
    }
    response->SendResponse();
  }
}
}

#endif // AVALON_WEBSERVICESESSION_HPP
