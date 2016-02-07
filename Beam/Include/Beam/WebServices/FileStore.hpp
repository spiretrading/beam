#ifndef BEAM_FILESTORE_HPP
#define BEAM_FILESTORE_HPP
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include "Beam/WebServices/HttpServerRequest.hpp"
#include "Beam/WebServices/HttpServerResponse.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class FileStore
      \brief Handles an HTTP request to serve a file.
   */
  class FileStore : private boost::noncopyable {
    public:

      //! Constructs a FileStore with the default current path.
      FileStore() = default;

      //! Constructs a FileStore with a specified path.
      /*!
        \param root The root of the file system.
      */
      FileStore(boost::filesystem::path root);

      //! Serves a file from an HTTP request.
      /*!
        \param request The HTTP request to serve.
        \return The HTTP response containing the file contents.
      */
      HttpServerResponse Serve(const HttpServerRequest& request);

    private:
      boost::filesystem::path m_root;
  };

  inline FileStore::FileStore(boost::filesystem::path root)
      : m_root{std::move(root)} {}

  inline HttpServerResponse FileStore::Serve(const HttpServerRequest& request) {
    boost::filesystem::path path = m_root / request.GetUri().GetPath();
    boost::filesystem::ifstream file{path, std::ios::in | std::ios::binary};
    HttpServerResponse response;
    if(!file) {
      response.SetStatusCode(HttpStatusCode::NOT_FOUND);
      return response;
    }
    IO::SharedBuffer buffer;
    buffer.Grow(static_cast<std::size_t>(boost::filesystem::file_size(path)));
    file.read(buffer.GetMutableData(), buffer.GetSize());
    response.SetBody(std::move(buffer));
    return response;
  }
}
}

#endif
