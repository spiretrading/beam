#ifndef BEAM_FILE_STORE_HPP
#define BEAM_FILE_STORE_HPP
#include <filesystem>
#include <fstream>
#include "Beam/Pointers/Out.hpp"
#include "Beam/WebServices/ContentTypePatterns.hpp"
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpRequestSlot.hpp"
#include "Beam/WebServices/HttpResponse.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam::WebServices {

  /** brief Handles an HTTP request to serve a file. */
  class FileStore {
    public:

      /**
       * Constructs a FileStore with a specified path.
       * @param root The root of the file system.
       */
      explicit FileStore(std::filesystem::path root);

      /**
       * Constructs a FileStore with a specified path.
       * @param root The root of the file system.
       * @param contentTypePatterns The set of patterns to use for content
       *        types.
       */
      FileStore(
        std::filesystem::path root, ContentTypePatterns contentTypePatterns);

      /**
       * Serves a file from a specified path.
       * @param path The path to the file to serve.
       * @return The HTTP response containing the file contents.
       */
      HttpResponse Serve(const std::filesystem::path& path);

      /**
       * Serves a file from an HTTP request.
       * @param request The HTTP request to serve.
       * @return The HTTP response containing the file contents.
       */
      HttpResponse Serve(const HttpRequest& request);

      /**
       * Serves a file from an HTTP request.
       * @param path The path to the file to serve.
       * @param response Stores the HTTP response containing the file contents.
       */
      void Serve(const std::filesystem::path& path, Out<HttpResponse> response);

      /**
       * Serves a file from an HTTP request.
       * @param request The HTTP request to serve.
       * @param response Stores the HTTP response containing the file contents.
       */
      void Serve(const HttpRequest& request, Out<HttpResponse> response);

    private:
      std::filesystem::path m_root;
      ContentTypePatterns m_contentTypePatterns;

      FileStore(const FileStore&) = delete;
      FileStore& operator =(const FileStore&) = delete;
      bool IsSubdirectory(const std::filesystem::path& target) const;
  };

  /**
   * Returns an HttpRequestSlot to serve index.html.
   * @param fileStore The FileStore serving the index.html file.
   * @return An HttpRequestSlot that serves index.html.
   */
  inline HttpRequestSlot ServeIndex(FileStore& fileStore) {
    return {[] (const HttpRequest& request) {
      return (request.GetUri().GetPath() == "/" ||
        request.GetUri().GetPath() == "") &&
        request.GetMethod() == HttpMethod::GET;
    },
    [&] (const HttpRequest& request) {
      auto response = HttpResponse();
      fileStore.Serve("index.html", Store(response));
      return response;
    }};
  }

  inline FileStore::FileStore(std::filesystem::path root)
    : FileStore(std::move(root), ContentTypePatterns::GetDefaultPatterns()) {}

  inline FileStore::FileStore(
    std::filesystem::path root, ContentTypePatterns contentTypePatterns)
    : m_root(std::filesystem::canonical(std::filesystem::absolute(root))),
      m_contentTypePatterns(std::move(contentTypePatterns)) {}

  inline HttpResponse FileStore::Serve(const std::filesystem::path& path) {
    auto response = HttpResponse();
    Serve(path, Store(response));
    return response;
  }

  inline HttpResponse FileStore::Serve(const HttpRequest& request) {
    if(!request.GetUri().GetPath().empty() &&
        request.GetUri().GetPath()[0] == '/') {
      return Serve(request.GetUri().GetPath().substr(1));
    } else {
      return Serve(request.GetUri().GetPath());
    }
  }

  inline void FileStore::Serve(
      const std::filesystem::path& path, Out<HttpResponse> response) {
    auto fullPath = std::filesystem::canonical(m_root / path);
    if(!IsSubdirectory(fullPath)) {
      response->SetStatusCode(HttpStatusCode::NOT_FOUND);
      return;
    }
    auto file = std::ifstream(fullPath, std::ios::in | std::ios::binary);
    if(!file) {
      response->SetStatusCode(HttpStatusCode::NOT_FOUND);
      return;
    }
    auto buffer = IO::SharedBuffer();
    buffer.Grow(static_cast<std::size_t>(std::filesystem::file_size(fullPath)));
    file.read(buffer.GetMutableData(), buffer.GetSize());
    auto& contentType = m_contentTypePatterns.GetContentType(fullPath);
    if(!contentType.empty()) {
      response->SetHeader({"Content-Type", contentType});
    }
    response->SetBody(std::move(buffer));
  }

  inline void FileStore::Serve(
      const HttpRequest& request, Out<HttpResponse> response) {
    Serve(request.GetUri().GetPath(), Store(response));
  }

  inline bool FileStore::IsSubdirectory(
      const std::filesystem::path& target) const {
    return std::mismatch(m_root.begin(), m_root.end(), target.begin()).first ==
      m_root.end();
  }
}

#endif
