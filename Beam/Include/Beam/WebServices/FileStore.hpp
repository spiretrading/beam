#ifndef BEAM_FILESTORE_HPP
#define BEAM_FILESTORE_HPP
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include "Beam/Pointers/Out.hpp"
#include "Beam/WebServices/ContentTypePatterns.hpp"
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

      //! Constructs a FileStore with a specified path.
      /*!
        \param root The root of the file system.
      */
      FileStore(boost::filesystem::path root);

      //! Constructs a FileStore with a specified path.
      /*!
        \param root The root of the file system.
        \param contentTypePatterns The set of patterns to use for content types.
      */
      FileStore(boost::filesystem::path root,
        ContentTypePatterns contentTypePatterns);

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

      //! Serves a file from an HTTP request.
      /*!
        \param path The path to the file to serve.
        \param response Stores the HTTP response containing the file contents.
      */
      void Serve(const boost::filesystem::path& path,
        Out<HttpServerResponse> response);

      //! Serves a file from an HTTP request.
      /*!
        \param request The HTTP request to serve.
        \param response Stores the HTTP response containing the file contents.
      */
      void Serve(const HttpServerRequest& request,
        Out<HttpServerResponse> response);

    private:
      boost::filesystem::path m_root;
      ContentTypePatterns m_contentTypePatterns;
  };

  inline FileStore::FileStore(boost::filesystem::path root)
      : m_contentTypePatterns{ContentTypePatterns::GetDefaultPatterns()} {
    m_root = boost::filesystem::canonical(boost::filesystem::absolute(root));
  }

  inline FileStore::FileStore(boost::filesystem::path root,
      ContentTypePatterns contentTypePatterns)
      : m_contentTypePatterns{std::move(contentTypePatterns)} {
    m_root = boost::filesystem::canonical(boost::filesystem::absolute(root));
  }

  inline HttpServerResponse FileStore::Serve(
      const boost::filesystem::path& path) {
    HttpServerResponse response;
    Serve(path, Store(response));
    return response;
  }

  inline HttpServerResponse FileStore::Serve(const HttpServerRequest& request) {
    return Serve(request.GetUri().GetPath());
  }

  inline void FileStore::Serve(const boost::filesystem::path& path,
      Out<HttpServerResponse> response) {
    boost::filesystem::path fullPath = m_root / path;
    boost::filesystem::ifstream file{fullPath, std::ios::in | std::ios::binary};
    if(!file) {
      response->SetStatusCode(HttpStatusCode::NOT_FOUND);
      return;
    }
    IO::SharedBuffer buffer;
    buffer.Grow(static_cast<std::size_t>(
      boost::filesystem::file_size(fullPath)));
    file.read(buffer.GetMutableData(), buffer.GetSize());
    auto& contentType = m_contentTypePatterns.GetContentType(fullPath);
    if(!contentType.empty()) {
      response->SetHeader({"Content-Type", contentType});
    }
    response->SetBody(std::move(buffer));
  }

  inline void FileStore::Serve(const HttpServerRequest& request,
      Out<HttpServerResponse> response) {
    Serve(request.GetUri().GetPath(), Store(response));
  }
}
}

#endif
