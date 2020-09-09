#ifndef BEAM_HTTP_SERVER_HPP
#define BEAM_HTTP_SERVER_HPP
#include <vector>
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

namespace Beam::WebServices {

  /**
   * Implements an HTTP server.
   * @param <C> The type of ServerConnection accepting Channels.
   */
  template<typename C>
  class HttpServer {
    public:

      /** The type of ServerConnection accepting Channels. */
      using ServerConnection = GetTryDereferenceType<C>;

      /** The type of Channel accepted by the ServerConnection. */
      using Channel = typename ServerConnection::Channel;

      /** The type of WebSocket used. */
      using WebSocket = Beam::WebServices::WebSocket<std::shared_ptr<Channel>>;

      /** The type of WebSocketChannel used. */
      using WebSocketChannel = Beam::WebServices::WebSocketChannel<
        std::shared_ptr<Channel>>;

      /** The type of slot used to upgrade to a WebSocket. */
      using WebSocketSlot =
        Beam::WebServices::HttpUpgradeSlot<WebSocketChannel>;

      /**
       * Constructs an HttpServer.
       * @param serverConnection Initializes the ServerConnection.
       * @param slots The slots handling the HttpServerRequests.
       */
      template<typename CF>
      HttpServer(CF&& serverConnection, std::vector<HttpRequestSlot> slots);

      /**
       * Constructs an HttpServer.
       * @param serverConnection Initializes the ServerConnection.
       * @param slots The slots handling the HttpServerRequests.
       * @param webSocketSlots The slots handling WebSocket upgrade requests.
       */
      template<typename CF>
      HttpServer(CF&& serverConnection, std::vector<HttpRequestSlot> slots,
        std::vector<WebSocketSlot> webSocketSlots);

      ~HttpServer();

      void Close();

    private:
      typename Channel::Writer::Buffer BAD_REQUEST_RESPONSE_BUFFER;
      typename Channel::Writer::Buffer NOT_FOUND_RESPONSE_BUFFER;
      GetOptionalLocalPtr<C> m_serverConnection;
      std::vector<HttpRequestSlot> m_slots;
      std::vector<WebSocketSlot> m_webSocketSlots;
      Routines::RoutineHandler m_acceptRoutine;
      IO::OpenState m_openState;

      HttpServer(const HttpServer&) = delete;
      HttpServer& operator =(const HttpServer&) = delete;
      void AcceptLoop();
      bool UpgradeConnection(const HttpRequest& request,
        const std::shared_ptr<Channel>& channel,
        typename Channel::Writer::Buffer& responseBuffer);
      bool HandleHttpRequest(const HttpRequest& request, Channel& channel,
        typename Channel::Writer::Buffer& responseBuffer);
  };

  template<typename C>
  template<typename CF>
  HttpServer<C>::HttpServer(CF&& serverConnection,
    std::vector<HttpRequestSlot> slots)
    : HttpServer(std::forward<CF>(serverConnection), std::move(slots),
        std::vector<WebSocketSlot>()) {}

  template<typename C>
  template<typename CF>
  HttpServer<C>::HttpServer(CF&& serverConnection,
      std::vector<HttpRequestSlot> slots,
      std::vector<WebSocketSlot> webSocketSlots)
      : m_serverConnection(std::forward<CF>(serverConnection)),
        m_slots(std::move(slots)),
        m_webSocketSlots(std::move(webSocketSlots)) {
    auto badRequestResponse = HttpResponse(HttpStatusCode::BAD_REQUEST);
    badRequestResponse.Encode(Store(BAD_REQUEST_RESPONSE_BUFFER));
    auto notFoundResponse = HttpResponse(HttpStatusCode::NOT_FOUND);
    notFoundResponse.Encode(Store(NOT_FOUND_RESPONSE_BUFFER));
    m_acceptRoutine = Routines::Spawn(std::bind(&HttpServer::AcceptLoop, this));
  }

  template<typename C>
  HttpServer<C>::~HttpServer() {
    Close();
  }

  template<typename C>
  void HttpServer<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_serverConnection->Close();
    m_acceptRoutine.Wait();
    m_openState.Close();
  }

  template<typename C>
  void HttpServer<C>::AcceptLoop() {
    auto clients = SynchronizedUnorderedSet<std::shared_ptr<Channel>>();
    auto clientRoutines = Routines::RoutineHandlerGroup();
    while(true) {
      auto channel = std::shared_ptr<Channel>();
      try {
        channel = m_serverConnection->Accept();
      } catch(const IO::EndOfFileException&) {
        break;
      } catch(const std::exception&) {
        std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        continue;
      }
      clients.Insert(channel);
      clientRoutines.Spawn([=, &clients] {
        auto parser = HttpRequestParser();
        auto requestBuffer = typename Channel::Reader::Buffer();
        auto responseBuffer = typename Channel::Writer::Buffer();
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
    auto pendingClients = std::unordered_set<std::shared_ptr<Channel>>();
    clients.Swap(pendingClients);
    for(auto& client : pendingClients) {
      client->GetConnection().Close();
    }
  }

  template<typename C>
  bool HttpServer<C>::UpgradeConnection(const HttpRequest& request,
      const std::shared_ptr<Channel>& channel,
      typename Channel::Writer::Buffer& responseBuffer) {
    static constexpr auto MAGIC_TOKEN = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
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
          auto response = HttpResponse(HttpStatusCode::SWITCHING_PROTOCOLS);
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

  template<typename C>
  bool HttpServer<C>::HandleHttpRequest(const HttpRequest& request,
      Channel& channel, typename Channel::Writer::Buffer& responseBuffer) {
    auto foundSlot = false;
    for(auto& slot : m_slots) {
      if(slot.m_predicate(request)) {
        try {
          auto response = slot.m_slot(request);
          response.Encode(Store(responseBuffer));
        } catch(const std::exception& e) {
          responseBuffer.Reset();
          auto response = HttpResponse(HttpStatusCode::INTERNAL_SERVER_ERROR);
          response.SetHeader({"Content-Type", "application/json"});
          auto jsonSender = Serialization::JsonSender<IO::SharedBuffer>();
          response.SetBody(Serialization::Encode<IO::SharedBuffer>(jsonSender,
            std::string(e.what())));
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

#endif
