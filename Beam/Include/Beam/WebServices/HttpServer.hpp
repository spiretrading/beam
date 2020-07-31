#ifndef BEAM_HTTPSERVER_HPP
#define BEAM_HTTPSERVER_HPP
#include <vector>
#include <boost/noncopyable.hpp>
#include "Beam/Collections/SynchronizedSet.hpp"
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Serialization/JsonSender.hpp"
#include "Beam/WebServices/HttpRequestParser.hpp"
#include "Beam/WebServices/HttpRequestSlot.hpp"
#include "Beam/WebServices/HttpResponse.hpp"
#include "Beam/WebServices/HttpUpgradeSlot.hpp"
#include "Beam/WebServices/WebSocketChannel.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class HttpServer
      \brief Implements an HTTP server.
      \tparam ServerConnectionType The type of ServerConnection accepting
              Channels.
   */
  template<typename ServerConnectionType>
  class HttpServer : private boost::noncopyable {
    public:

      //! The type of ServerConnection accepting Channels.
      using ServerConnection = GetTryDereferenceType<ServerConnectionType>;

      //! The type of Channel accepted by the ServerConnection.
      using Channel = typename ServerConnection::Channel;

      //! The type of WebSocket used.
      using WebSocket =
        ::Beam::WebServices::WebSocket<std::shared_ptr<Channel>>;

      //! The type of WebSocketChannel used.
      using WebSocketChannel = ::Beam::WebServices::WebSocketChannel<
        std::shared_ptr<Channel>>;

      //! The type of slot used to upgrade to a WebSocket.
      using WebSocketSlot =
        ::Beam::WebServices::HttpUpgradeSlot<WebSocketChannel>;

      //! Constructs an HttpServer.
      /*!
        \param serverConnection Initializes the ServerConnection.
        \param slots The slots handling the HttpServerRequests.
      */
      template<typename ServerConnectionForward>
      HttpServer(ServerConnectionForward&& serverConnection,
        std::vector<HttpRequestSlot> slots);

      //! Constructs an HttpServer.
      /*!
        \param serverConnection Initializes the ServerConnection.
        \param slots The slots handling the HttpServerRequests.
        \param webSocketSlots The slots handling WebSocket upgrade requests.
      */
      template<typename ServerConnectionForward>
      HttpServer(ServerConnectionForward&& serverConnection,
        std::vector<HttpRequestSlot> slots,
        std::vector<WebSocketSlot> webSocketSlots);

      ~HttpServer();

      void Open();

      void Close();

    private:
      typename Channel::Writer::Buffer BAD_REQUEST_RESPONSE_BUFFER;
      typename Channel::Writer::Buffer NOT_FOUND_RESPONSE_BUFFER;
      GetOptionalLocalPtr<ServerConnectionType> m_serverConnection;
      std::vector<HttpRequestSlot> m_slots;
      std::vector<WebSocketSlot> m_webSocketSlots;
      Routines::RoutineHandler m_acceptRoutine;
      IO::OpenState m_openState;

      void Shutdown();
      void AcceptLoop();
      bool UpgradeConnection(const HttpRequest& request,
        const std::shared_ptr<Channel>& channel,
        typename Channel::Writer::Buffer& responseBuffer);
      bool HandleHttpRequest(const HttpRequest& request, Channel& channel,
        typename Channel::Writer::Buffer& responseBuffer);
  };

  template<typename ServerConnectionType>
  template<typename ServerConnectionForward>
  HttpServer<ServerConnectionType>::HttpServer(
      ServerConnectionForward&& serverConnection,
      std::vector<HttpRequestSlot> slots)
      : HttpServer{std::forward<ServerConnectionForward>(serverConnection),
          std::move(slots), std::vector<WebSocketSlot>()} {}

  template<typename ServerConnectionType>
  template<typename ServerConnectionForward>
  HttpServer<ServerConnectionType>::HttpServer(
      ServerConnectionForward&& serverConnection,
      std::vector<HttpRequestSlot> slots,
      std::vector<WebSocketSlot> webSocketSlots)
      : m_serverConnection{std::forward<ServerConnectionForward>(
          serverConnection)},
        m_slots{std::move(slots)},
        m_webSocketSlots{std::move(webSocketSlots)} {
    HttpResponse badRequestResponse{HttpStatusCode::BAD_REQUEST};
    badRequestResponse.Encode(Store(BAD_REQUEST_RESPONSE_BUFFER));
    HttpResponse notFoundResponse{HttpStatusCode::NOT_FOUND};
    notFoundResponse.Encode(Store(NOT_FOUND_RESPONSE_BUFFER));
  }

  template<typename ServerConnectionType>
  HttpServer<ServerConnectionType>::~HttpServer() {
    Close();
  }

  template<typename ServerConnectionType>
  void HttpServer<ServerConnectionType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_serverConnection->Open();
      m_acceptRoutine = Routines::Spawn(
        std::bind(&HttpServer::AcceptLoop, this));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ServerConnectionType>
  void HttpServer<ServerConnectionType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ServerConnectionType>
  void HttpServer<ServerConnectionType>::Shutdown() {
    m_serverConnection->Close();
    m_acceptRoutine.Wait();
    m_openState.SetClosed();
  }

  template<typename ServerConnectionType>
  void HttpServer<ServerConnectionType>::AcceptLoop() {
    SynchronizedUnorderedSet<std::shared_ptr<Channel>> clients;
    Routines::RoutineHandlerGroup clientRoutines;
    while(true) {
      std::shared_ptr<Channel> channel;
      try {
        channel = m_serverConnection->Accept();
      } catch(const IO::EndOfFileException&) {
        break;
      } catch(const std::exception&) {
        std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        continue;
      }
      clients.Insert(channel);
      clientRoutines.Spawn(
        [=, &clients] {
          try {
            channel->GetConnection().Open();
          } catch(const std::exception&) {
            std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
            clients.Erase(channel);
            return;
          }
          HttpRequestParser parser;
          typename Channel::Reader::Buffer requestBuffer;
          typename Channel::Writer::Buffer responseBuffer;
          try {
            while(true) {
              channel->GetReader().Read(Store(requestBuffer));
              parser.Feed(requestBuffer.GetData(), requestBuffer.GetSize());
              requestBuffer.Reset();
              auto request = parser.GetNextRequest();
              while(request.is_initialized()) {
                std::cout << *request << "\n\n\n" << std::flush;
                responseBuffer.Reset();
                if(request->GetSpecialHeaders().m_connection ==
                    ConnectionHeader::UPGRADE) {
                  auto wasUpgraded = UpgradeConnection(*request, channel,
                    responseBuffer);
                  if(wasUpgraded) {
                    clients.Erase(channel);
                    return;
                  }
                } else {
                  auto keepAlive = HandleHttpRequest(*request, *channel,
                    responseBuffer);
                  if(!keepAlive) {
                    clients.Erase(channel);
                    channel->GetConnection().Close();
                    return;
                  }
                }
                request = parser.GetNextRequest();
              }
            }
          } catch(const std::exception&) {}
          clients.Erase(channel);
        });
    }
    std::unordered_set<std::shared_ptr<Channel>> pendingClients;
    clients.Swap(pendingClients);
    for(auto& client : pendingClients) {
      client->GetConnection().Close();
    }
  }

  template<typename ServerConnectionType>
  bool HttpServer<ServerConnectionType>::UpgradeConnection(
      const HttpRequest& request, const std::shared_ptr<Channel>& channel,
      typename Channel::Writer::Buffer& responseBuffer) {
    static auto MAGIC_TOKEN = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    auto protocol = request.GetHeader("Upgrade");
    if(!protocol.is_initialized()) {
      channel->GetWriter().Write(BAD_REQUEST_RESPONSE_BUFFER);
      return false;
    }
    if(*protocol == "websocket") {
      auto key = request.GetHeader("Sec-WebSocket-Key");
      if(!key.is_initialized()) {
        channel->GetWriter().Write(BAD_REQUEST_RESPONSE_BUFFER);
        return false;
      }
      auto acceptToken = IO::Base64Encode(
        IO::BufferFromString<IO::SharedBuffer>(Details::ComputeShaDigest(
        *key + MAGIC_TOKEN)));
      for(auto& slot : m_webSocketSlots) {
        if(slot.m_predicate(request)) {
          HttpResponse response{HttpStatusCode::SWITCHING_PROTOCOLS};
          response.SetHeader({"Connection", "Upgrade"});
          response.SetHeader({"Upgrade", "websocket"});
          response.SetHeader({"Sec-WebSocket-Accept", acceptToken});
          response.Encode(Store(responseBuffer));
          channel->GetWriter().Write(responseBuffer);
          auto webSocket = std::make_unique<WebSocket>(channel,
            typename WebSocket::ServerTag{});
          auto webSocketChannel = std::make_unique<WebSocketChannel>(
            std::move(webSocket));
          slot.m_slot(request, std::move(webSocketChannel));
          return true;
        }
      }
    }
    channel->GetWriter().Write(NOT_FOUND_RESPONSE_BUFFER);
    return false;
  }

  template<typename ServerConnectionType>
  bool HttpServer<ServerConnectionType>::HandleHttpRequest(
      const HttpRequest& request, Channel& channel,
      typename Channel::Writer::Buffer& responseBuffer) {
    auto foundSlot = false;
    for(auto& slot : m_slots) {
      if(slot.m_predicate(request)) {
        try {
          auto response = slot.m_slot(request);
          response.Encode(Store(responseBuffer));
        } catch(const std::exception& e) {
          responseBuffer.Reset();
          HttpResponse response{HttpStatusCode::INTERNAL_SERVER_ERROR};
          response.SetHeader({"Content-Type", "application/json"});
          Serialization::JsonSender<IO::SharedBuffer> jsonSender;
          response.SetBody(Serialization::Encode<IO::SharedBuffer>(jsonSender,
            std::string{e.what()}));
          response.Encode(Store(responseBuffer));
        }
        channel.GetWriter().Write(responseBuffer);
        foundSlot = true;
        break;
      }
    }
    if(!foundSlot) {
      channel.GetWriter().Write(NOT_FOUND_RESPONSE_BUFFER);
    }
    return request.GetSpecialHeaders().m_connection != ConnectionHeader::CLOSE;
  }
}
}

#endif
