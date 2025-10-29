#ifndef BEAM_HTTP_FILE_SERVLET_HPP
#define BEAM_HTTP_FILE_SERVLET_HPP
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/FileStore.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>

namespace Beam {

  /** Implements a web servlet for static files. */
  class HttpFileServlet {
    public:

      /** Constructs a HttpFileServlet. */
      HttpFileServlet();

      ~HttpFileServlet();

      std::vector<HttpRequestSlot> get_slots();
      void close();

    private:
      FileStore m_file_store;
      OpenState m_open_state;

      HttpFileServlet(const HttpFileServlet&) = delete;
      HttpFileServlet& operator =(const HttpFileServlet&) = delete;
      HttpResponse on_serve_file(const HttpRequest& request);
  };
}

#endif
