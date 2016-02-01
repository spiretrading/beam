#ifndef AVALON_HTTPPROTOCOL_HPP
#define AVALON_HTTPPROTOCOL_HPP
#include <deque>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include "Avalon/IO/Connection.hpp"
#include "Avalon/Threading/Async.hpp"
#include "Avalon/WebServices/HttpRequestParser.hpp"
#include "Avalon/WebServices/WebServices.hpp"

namespace Avalon {
namespace WebServices {

  /*! \class HttpProtocol
      \brief Implements the HTTP protocol.
   */
  class HttpProtocol {
    public:

      //! The type of slot called when the Connection closes.
      typedef IO::Connection::ClosedSignal::slot_function_type ClosedSlot;

      //! The amount of time to wait for a request/response.
      static const boost::posix_time::time_duration KEEP_ALIVE;

      //! Constructs an HttpProtocol.
      HttpProtocol();

      ~HttpProtocol();

      //! Initializes the HttpProtocol.
      /*!
        \param channel The Channel to implement the protocol for.
        \param closedSlot The slot to call when the Channel closes.
        \param requestSlots The list of slots to call in response to an HTTP
                            request.
        \param keepAliveTimer Keeps track of the keep-alive.
      */
      void Initialize(IO::Channel* channel, const ClosedSlot& closedSlot,
        std::vector<HttpRequestSlot>* requestSlots,
        Threading::Timer* keepAliveTimer);

      //! Returns the Channel.
      IO::Channel& GetChannel();

      //! Starts the protocol.
      void Start();

      //! Stops the protocol.
      void Stop();

      //! Sends a response to the client.
      /*!
        \param response The response to the client.
      */
      void SendResponse(HttpServerResponse* response);

    private:
      friend class HttpServerResponse;
      boost::mutex m_mutex;
      IO::Channel* m_channel;
      int m_timeoutCounter;
      boost::scoped_ptr<Threading::Timer> m_keepAliveTimer;
      bool m_keepAliveActive;
      bool m_isStarted;
      bool m_isStopping;
      HttpRequestParser m_requestParser;
      std::vector<HttpRequestSlot>* m_requestSlots;
      Threading::Async<int> m_readResult;
      Threading::Async<int>::State m_readState;
      Threading::Async<int>::FinishedSlot m_writeSlot;
      Threading::Async<int>::FinishedSlot m_readSlot;
      std::deque<Threading::Async<void>*> m_pendingWrites;
      boost::condition_variable m_operationCompleteCondition;
      ClosedSlot m_closedSlot;

      void CloseResources(boost::unique_lock<boost::mutex>& lock);
      void WriteToChannel(const IO::Buffer& buffer,
        boost::unique_lock<boost::mutex>& lock);
      void OnRead();
      void OnWrite();
      void OnKeepAliveExpiry();
  };
}
}

#endif // AVALON_HTTPPROTOCOL_HPP
