#ifndef BEAM_HTTP_SERVER_HPP
#define BEAM_HTTP_SERVER_HPP
#include <vector>
#include "Beam/Collections/SynchronizedSet.hpp"
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Serialization/JsonSender.hpp"
#include "Beam/Utilities/TypeTraits.hpp"
#include "Beam/WebServices/HttpRequestParser.hpp"
#include "Beam/WebServices/HttpRequestSlot.hpp"
#include "Beam/WebServices/HttpResponse.hpp"
#include "Beam/WebServices/HttpUpgradeSlot.hpp"
#include "Beam/WebServices/WebSocketChannel.hpp"

namespace Beam {
namespace Details {
  inline ConnectionHeader get_special_headers_connection(
      const HttpRequest& request) {
    return request.get_special_headers().m_connection;
  }
}

  /**
   * Implements an HTTP server.
   * @tparam C The type of ServerConnection accepting Channels.
   */
  template<typename C> requires IsServerConnection<dereference_t<C>>
  class HttpServer {
    public:

      /** The type of ServerConnection accepting Channels. */
      using ServerConnection = dereference_t<C>;

      /** The type of Channel accepted by the ServerConnection. */
      using Channel = typename ServerConnection::Channel;

      /** The type of WebSocket used. */
      using WebSocket = Beam::WebSocket<std::shared_ptr<Channel>>;

      /** The type of WebSocketChannel used. */
      using WebSocketChannel = Beam::WebSocketChannel<std::shared_ptr<Channel>>;

      /** The type of slot used to upgrade to a WebSocket. */
      using WebSocketSlot = Beam::HttpUpgradeSlot<WebSocketChannel>;

      /**
       * Constructs an HttpServer.
       * @param server_connection Initializes the ServerConnection.
       * @param slots The slots handling the HttpServerRequests.
       */
      template<Initializes<C> CF>
      HttpServer(CF&& server_connection, std::vector<HttpRequestSlot> slots);

      /**
       * Constructs an HttpServer.
       * @param server_connection Initializes the ServerConnection.
       * @param slots The slots handling the HttpServerRequests.
       * @param web_socket_slots The slots handling WebSocket upgrade requests.
       */
      template<Initializes<C> CF>
      HttpServer(CF&& server_connection, std::vector<HttpRequestSlot> slots,
        std::vector<WebSocketSlot> web_socket_slots);

      ~HttpServer();

      void close();

    private:
      SharedBuffer BAD_REQUEST_RESPONSE_BUFFER;
      SharedBuffer NOT_FOUND_RESPONSE_BUFFER;
      local_ptr_t<C> m_server_connection;
      std::vector<HttpRequestSlot> m_slots;
      std::vector<WebSocketSlot> m_web_socket_slots;
      RoutineHandler m_accept_routine;
      OpenState m_open_state;

      HttpServer(const HttpServer&) = delete;
      HttpServer& operator =(const HttpServer&) = delete;
      void accept_loop();
      bool upgrade_connection(const HttpRequest& request,
        const std::shared_ptr<Channel>& channel, SharedBuffer& response_buffer);
      bool handle_http_request(const HttpRequest& request, Channel& channel,
        SharedBuffer& response_buffer);
  };

  template<typename C>
  HttpServer(C&&, std::vector<HttpRequestSlot>) ->
    HttpServer<std::remove_cvref_t<C>>;

  template<typename C>
  HttpServer(C&&, std::vector<HttpRequestSlot>,
    std::vector<typename HttpServer<std::remove_cvref_t<C>>::WebSocketSlot>) ->
      HttpServer<std::remove_cvref_t<C>>;

  template<typename C> requires IsServerConnection<dereference_t<C>>
  template<Initializes<C> CF>
  HttpServer<C>::HttpServer(
    CF&& server_connection, std::vector<HttpRequestSlot> slots)
    : HttpServer(std::forward<CF>(server_connection), std::move(slots), {}) {}

  template<typename C> requires IsServerConnection<dereference_t<C>>
  template<Initializes<C> CF>
  HttpServer<C>::HttpServer(CF&& server_connection,
      std::vector<HttpRequestSlot> slots,
      std::vector<WebSocketSlot> web_socket_slots)
      : m_server_connection(std::forward<CF>(server_connection)),
        m_slots(std::move(slots)),
        m_web_socket_slots(std::move(web_socket_slots)) {
    auto bad_request_response = HttpResponse(HttpStatusCode::BAD_REQUEST);
    bad_request_response.encode(out(BAD_REQUEST_RESPONSE_BUFFER));
    auto not_found_response = HttpResponse(HttpStatusCode::NOT_FOUND);
    not_found_response.encode(out(NOT_FOUND_RESPONSE_BUFFER));
    m_accept_routine = spawn(std::bind_front(&HttpServer::accept_loop, this));
  }

  template<typename C> requires IsServerConnection<dereference_t<C>>
  HttpServer<C>::~HttpServer() {
    close();
  }

  template<typename C> requires IsServerConnection<dereference_t<C>>
  void HttpServer<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_server_connection->close();
    m_accept_routine.wait();
    m_open_state.close();
  }

  template<typename C> requires IsServerConnection<dereference_t<C>>
  void HttpServer<C>::accept_loop() {
    auto clients = SynchronizedUnorderedSet<std::shared_ptr<Channel>>();
    auto client_routines = RoutineHandlerGroup();
    while(true) {
      auto channel = std::shared_ptr<Channel>();
      try {
        channel = m_server_connection->accept();
      } catch(const EndOfFileException&) {
        break;
      } catch(const std::exception&) {
        std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        continue;
      }
      clients.insert(channel);
      client_routines.spawn([=, this, &clients] {
        auto parser = HttpRequestParser();
        auto request_buffer = SharedBuffer();
        auto response_buffer = SharedBuffer();
        try {
          while(true) {
            channel->get_reader().read(out(request_buffer));
            parser.feed(std::string_view(
              request_buffer.get_data(), request_buffer.get_size()));
            reset(request_buffer);
            auto request = parser.get_next_request();
            while(request) {
              reset(response_buffer);
              auto connection =
                Details::get_special_headers_connection(*request);
              if(connection == ConnectionHeader::UPGRADE) {
                auto was_upgraded =
                  upgrade_connection(*request, channel, response_buffer);
                if(was_upgraded) {
                  clients.erase(channel);
                  return;
                }
              } else {
                auto keep_alive =
                  handle_http_request(*request, *channel, response_buffer);
                if(!keep_alive) {
                  clients.erase(channel);
                  channel->get_connection().close();
                  return;
                }
              }
              request = parser.get_next_request();
            }
          }
        } catch(const std::exception&) {}
        clients.erase(channel);
      });
    }
    auto pending_clients = std::unordered_set<std::shared_ptr<Channel>>();
    clients.swap(pending_clients);
    for(auto& client : pending_clients) {
      client->get_connection().close();
    }
  }

  template<typename C> requires IsServerConnection<dereference_t<C>>
  bool HttpServer<C>::upgrade_connection(const HttpRequest& request,
      const std::shared_ptr<Channel>& channel, SharedBuffer& response_buffer) {
    static constexpr auto MAGIC_TOKEN = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    auto protocol = request.get_header("Upgrade");
    if(!protocol) {
      channel->get_writer().write(BAD_REQUEST_RESPONSE_BUFFER);
      return false;
    }
    if(*protocol == "websocket") {
      auto key = request.get_header("Sec-WebSocket-Key");
      if(!key) {
        channel->get_writer().write(BAD_REQUEST_RESPONSE_BUFFER);
        return false;
      }
      auto accept_token = encode_base64(from<SharedBuffer>(
        Details::compute_sha_digest(*key + MAGIC_TOKEN)));
      for(auto& slot : m_web_socket_slots) {
        if(slot.m_predicate(request)) {
          auto response = HttpResponse(HttpStatusCode::SWITCHING_PROTOCOLS);
          response.set_header({"Connection", "Upgrade"});
          response.set_header({"Upgrade", "websocket"});
          response.set_header({"Sec-WebSocket-Accept", accept_token});
          response.encode(out(response_buffer));
          channel->get_writer().write(response_buffer);
          auto web_socket = std::make_unique<WebSocket>(
            channel, typename WebSocket::ServerTag{});
          auto web_socket_channel =
            std::make_unique<WebSocketChannel>(std::move(web_socket));
          slot.m_slot(request, std::move(web_socket_channel));
          return true;
        }
      }
    }
    channel->get_writer().write(NOT_FOUND_RESPONSE_BUFFER);
    return false;
  }

  template<typename C> requires IsServerConnection<dereference_t<C>>
  bool HttpServer<C>::handle_http_request(const HttpRequest& request,
      Channel& channel, SharedBuffer& response_buffer) {
    auto found_slot = false;
    for(auto& slot : m_slots) {
      if(slot.m_predicate(request)) {
        try {
          auto response = slot.m_slot(request);
          response.encode(out(response_buffer));
        } catch(const std::exception& e) {
          reset(response_buffer);
          auto response = HttpResponse(HttpStatusCode::INTERNAL_SERVER_ERROR);
          response.set_header({"Content-Type", "application/json"});
          auto json_sender = JsonSender<SharedBuffer>();
          response.set_body(
            encode<SharedBuffer>(json_sender, std::string(e.what())));
          response.encode(out(response_buffer));
        }
        channel.get_writer().write(response_buffer);
        found_slot = true;
        break;
      }
    }
    if(!found_slot) {
      channel.get_writer().write(NOT_FOUND_RESPONSE_BUFFER);
    }
    return request.get_special_headers().m_connection !=
      ConnectionHeader::CLOSE;
  }
}

#endif
