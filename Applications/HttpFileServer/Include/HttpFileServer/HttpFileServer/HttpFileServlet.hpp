#ifndef BEAM_HTTPFILESERVLET_HPP
#define BEAM_HTTPFILESERVLET_HPP
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/WebServices/FileStore.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <boost/noncopyable.hpp>
#include "HttpFileServer/HttpFileServer/HttpFileServer.hpp"

namespace Beam {
namespace HttpFileServer {

  /*! \class HttpFileServlet
      \brief Implements a web servlet for static files.
   */
  class HttpFileServlet : private boost::noncopyable {
    public:

      //! Constructs a HttpFileServlet.
      HttpFileServlet();

      ~HttpFileServlet();

      //! Returns the HTTP request slots.
      std::vector<Beam::WebServices::HttpRequestSlot> GetSlots();

      void Open();

      void Close();

    private:
      Beam::WebServices::FileStore m_fileStore;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      Beam::WebServices::HttpResponse OnServeFile(
        const Beam::WebServices::HttpRequest& request);
  };
}
}

#endif
