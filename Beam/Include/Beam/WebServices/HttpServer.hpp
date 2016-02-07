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
#include "Beam/WebServices/HttpServerResponse.hpp"
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
      GetOptionalLocalPtr<ServerConnectionType> m_serverConnection;
      std::vector<HttpRequestSlot> m_slots;
      Routines::RoutineHandler m_acceptRoutine;
      IO::OpenState m_openState;

      void Shutdown();
      void AcceptLoop();
  };

  template<typename ServerConnectionType>
  template<typename ServerConnectionForward>
  HttpServer<ServerConnectionType>::HttpServer(
      ServerConnectionForward&& serverConnection,
      std::vector<HttpRequestSlot> slots)
      : m_serverConnection{std::forward<ServerConnectionForward>(
          serverConnection)},
        m_slots{std::move(slots)} {}

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
    HttpServerResponse NOT_FOUND_RESPONSE{HttpStatusCode::NOT_FOUND};
    typename Channel::Writer::Buffer NOT_FOUND_RESPONSE_BUFFER;
    NOT_FOUND_RESPONSE.Encode(Store(NOT_FOUND_RESPONSE_BUFFER));
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
          try {
            HttpRequestParser parser;
            typename Channel::Reader::Buffer requestBuffer;
            typename Channel::Writer::Buffer responseBuffer;
            while(true) {
              channel->GetReader().Read(Store(requestBuffer));
              parser.Feed(requestBuffer.GetData(), requestBuffer.GetSize());
              requestBuffer.Reset();
              auto request = parser.GetNextRequest();
              while(request.is_initialized()) {
                auto foundSlot = false;
                for(auto& slot : m_slots) {
                  if(slot.m_predicate(*request)) {
                    auto response = slot.m_slot(*request);
                    response.Encode(Store(responseBuffer));
                    channel->GetWriter().Write(responseBuffer);
                    responseBuffer.Reset();
                    foundSlot = true;
                    break;
                  }
                }
                if(!foundSlot) {
                  channel->GetWriter().Write(NOT_FOUND_RESPONSE_BUFFER);
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
}
}

#endif
