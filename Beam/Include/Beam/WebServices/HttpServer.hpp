#ifndef BEAM_HTTPSERVER_HPP
#define BEAM_HTTPSERVER_HPP
#include <vector>
#include <boost/noncopyable.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Utilities/SynchronizedSet.hpp"
#include "Beam/WebServices/HttpRequestParser.hpp"
#include "Beam/WebServices/HttpRequestSlot.hpp"
#include "Beam/WebServices/HttpResponse.hpp"
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

      //! Constructs an HttpServer.
      /*!
        \param serverConnection Initializes the ServerConnection.
        \param slots The slots handling the HttpServerRequests.
      */
      template<typename ServerConnectionForward>
      HttpServer(ServerConnectionForward&& serverConnection,
        std::vector<HttpRequestSlot> slots);

      ~HttpServer();

      void Open();

      void Close();

    private:
      enum class Protocol {
        CLOSE = 0,
        HTTP,
        WEB_SOCKET
      };
      typename Channel::Writer::Buffer BAD_REQUEST_RESPONSE_BUFFER;
      typename Channel::Writer::Buffer NOT_FOUND_RESPONSE_BUFFER;
      GetOptionalLocalPtr<ServerConnectionType> m_serverConnection;
      std::vector<HttpRequestSlot> m_slots;
      Routines::RoutineHandler m_acceptRoutine;
      IO::OpenState m_openState;

      void Shutdown();
      void AcceptLoop();
      Protocol HttpRequestLoop(Channel& channel);
      Protocol WebSocketLoop(Channel& channel);
  };

  template<typename ServerConnectionType>
  template<typename ServerConnectionForward>
  HttpServer<ServerConnectionType>::HttpServer(
      ServerConnectionForward&& serverConnection,
      std::vector<HttpRequestSlot> slots)
      : m_serverConnection{std::forward<ServerConnectionForward>(
          serverConnection)},
        m_slots{std::move(slots)} {
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
            return;
          }
          auto protocol = Protocol::HTTP;
          while(protocol != Protocol::CLOSE) {
            if(protocol == Protocol::HTTP) {
              protocol = HttpRequestLoop(*channel);
            } else if(protocol == Protocol::WEB_SOCKET) {
              protocol = WebSocketLoop(*channel);
            } else {
              protocol = Protocol::CLOSE;
            }
          }
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
  typename HttpServer<ServerConnectionType>::Protocol
      HttpServer<ServerConnectionType>::HttpRequestLoop(Channel& channel) {
    try {
      HttpRequestParser parser;
      typename Channel::Reader::Buffer requestBuffer;
      typename Channel::Writer::Buffer responseBuffer;
      while(true) {
        channel.GetReader().Read(Store(requestBuffer));
        parser.Feed(requestBuffer.GetData(), requestBuffer.GetSize());
        requestBuffer.Reset();
        auto request = parser.GetNextRequest();
        while(request.is_initialized()) {
          std::cout << *request << "\n\n\n" << std::flush;
          if(request->GetSpecialHeaders().m_connection ==
              ConnectionHeader::UPGRADE) {
            auto protocol = request->GetHeader("Upgrade");
            if(!protocol.is_initialized()) {
              channel.GetWriter().Write(BAD_REQUEST_RESPONSE_BUFFER);
            } else if(*protocol == "websocket") {
              auto foundSlot = false;
              auto accepted = false;
              for(auto& slot : m_slots) {
                if(slot.m_predicate(*request)) {
                  auto response = slot.m_slot(*request);
                  if(response.GetStatusCode() ==
                      HttpStatusCode::SWITCHING_PROTOCOLS) {
                    accepted = true;

                    // TODO: Add proper response headers for websocket.
                  }
                  response.Encode(Store(responseBuffer));
                  channel.GetWriter().Write(responseBuffer);
                  responseBuffer.Reset();
                  foundSlot = true;
                  break;
                }
              }
              if(!foundSlot) {
                channel.GetWriter().Write(NOT_FOUND_RESPONSE_BUFFER);
              }
              if(accepted) {
                return Protocol::WEB_SOCKET;
              }
            }
          } else {
            auto foundSlot = false;
            for(auto& slot : m_slots) {
              if(slot.m_predicate(*request)) {
                auto response = slot.m_slot(*request);
                response.Encode(Store(responseBuffer));
                channel.GetWriter().Write(responseBuffer);
                responseBuffer.Reset();
                foundSlot = true;
                break;
              }
            }
            if(!foundSlot) {
              channel.GetWriter().Write(NOT_FOUND_RESPONSE_BUFFER);
            }
            if(request->GetSpecialHeaders().m_connection ==
                ConnectionHeader::CLOSE) {
              channel.GetConnection().Close();
              break;
            } else {
              request = parser.GetNextRequest();
            }
          }
        }
      }
    } catch(const std::exception&) {}
    return Protocol::CLOSE;
  }

  template<typename ServerConnectionType>
  typename HttpServer<ServerConnectionType>::Protocol
      HttpServer<ServerConnectionType>::WebSocketLoop(Channel& channel) {
    return Protocol::CLOSE;
  }
}
}

#endif
