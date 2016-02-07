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
      FileStore();

      //! Constructs a FileStore with a specified path.
      /*!
        \param root The root of the file system.
      */
      FileStore(boost::filesystem::path root);

      //! Serves a file from a specified path.
      /*!
        \param path The path to the file to serve.
        \return The HTTP response containing the file contents.
      */
      HttpServerResponse Serve(const boost::filesystem::path& path);

      //! Serves a file from an HTTP request.
      /*!
        \param request The HTTP request to serve.
        \return The HTTP response containing the file contents.
      */
      HttpServerResponse Serve(const HttpServerRequest& request);

    private:
      boost::filesystem::path m_root;
  };

  inline FileStore::FileStore()
      : FileStore{boost::filesystem::current_path()} {}

  inline FileStore::FileStore(boost::filesystem::path root) {
    m_root = boost::filesystem::canonical(boost::filesystem::absolute(root));
  }

  inline HttpServerResponse FileStore::Serve(
      const boost::filesystem::path& path) {
    boost::filesystem::path fullPath = m_root / path;
    boost::filesystem::ifstream file{fullPath, std::ios::in | std::ios::binary};
    HttpServerResponse response;
    if(!file) {
      response.SetStatusCode(HttpStatusCode::NOT_FOUND);
      return response;
    }
    IO::SharedBuffer buffer;
    buffer.Grow(static_cast<std::size_t>(
      boost::filesystem::file_size(fullPath)));
    file.read(buffer.GetMutableData(), buffer.GetSize());
    response.SetBody(std::move(buffer));
    return response;
  }

  inline HttpServerResponse FileStore::Serve(const HttpServerRequest& request) {
    return Serve(request.GetUri().GetPath());
  }
}
}

#endif
