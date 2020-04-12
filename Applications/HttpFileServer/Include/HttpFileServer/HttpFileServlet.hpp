#ifndef BEAM_HTTP_FILE_SERVLET_HPP
#define BEAM_HTTP_FILE_SERVLET_HPP
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/FileStore.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <boost/noncopyable.hpp>
#include "HttpFileServer/HttpFileServer.hpp"

namespace Beam::HttpFileServer {

  /** Implements a web servlet for static files. */
  class HttpFileServlet : private boost::noncopyable {
    public:

      /** Constructs a HttpFileServlet. */
      HttpFileServlet();

      ~HttpFileServlet();

      std::vector<WebServices::HttpRequestSlot> GetSlots();

      void Open();

      void Close();

    private:
      WebServices::FileStore m_fileStore;
      IO::OpenState m_openState;

      void Shutdown();
      WebServices::HttpResponse OnServeFile(
        const WebServices::HttpRequest& request);
  };
}

#endif
