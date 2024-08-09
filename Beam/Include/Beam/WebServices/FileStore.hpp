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
      static void Populate(std::ostream& out, const std::string& name);
      static std::string MakeDisplayPath(const std::filesystem::path& path);
      std::string MakeDirectoryListing(
        const std::filesystem::path& target) const;
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
    auto errorCode = std::error_code();
    auto fullPath = std::filesystem::canonical(m_root / path, errorCode);
    if(errorCode || !IsSubdirectory(fullPath)) {
      response->SetStatusCode(HttpStatusCode::NOT_FOUND);
      return;
    }
    auto buffer = IO::SharedBuffer();
    if(std::filesystem::is_directory(fullPath)) {
      auto listing = MakeDirectoryListing(fullPath);
      buffer.Append(listing.c_str(), listing.size());
      response->SetHeader({"Content-Type", "text/html"});
    } else {
      auto file = std::ifstream(fullPath, std::ios::in | std::ios::binary);
      if(!file) {
        response->SetStatusCode(HttpStatusCode::NOT_FOUND);
        return;
      }
      buffer.Grow(
        static_cast<std::size_t>(std::filesystem::file_size(fullPath)));
      file.read(buffer.GetMutableData(), buffer.GetSize());
      auto& contentType = m_contentTypePatterns.GetContentType(fullPath);
      if(!contentType.empty()) {
        response->SetHeader({"Content-Type", contentType});
      }
    }
    response->SetBody(std::move(buffer));
  }

  inline void FileStore::Serve(
      const HttpRequest& request, Out<HttpResponse> response) {
    Serve(request.GetUri().GetPath(), Store(response));
  }

  inline void FileStore::Populate(std::ostream& out, const std::string& name) {
    out << "<a href=\"" << name << "\">" << name << "</a>\n";
  }

  inline std::string FileStore::MakeDisplayPath(
      const std::filesystem::path& path) {
    auto out = std::string();
    for(auto i = path.begin(); i != path.end(); ++i) {
      out += i->string();
      if(std::next(i) != path.end()) {
        out += '/';
      }
    }
    return out;
  }

  inline std::string FileStore::MakeDirectoryListing(
      const std::filesystem::path& target) const {
    auto relative_path = std::filesystem::relative(target, m_root);
    auto display_path = [&] {
      if(target == m_root) {
        return std::string("/");
      } else {
        return "/" + MakeDisplayPath(relative_path);
      }
    }();
    auto out = std::ostringstream();
    out << "<!DOCTYPE html>\n";
    out << "<html>\n<head>\n<title>Index of " << display_path <<
      "</title>\n</head>\n<body>\n";
    out << "<h1>Index of " << display_path << "</h1>\n";
    out << "<hr>\n<pre>\n";
    if(target != m_root) {
      out << "<a href=\"../\">..</a>\n";
    }
    auto directories = std::vector<std::string>();
    auto files = std::vector<std::string>();
    for(auto& entry : std::filesystem::directory_iterator(target)) {
      if(entry.is_directory()) {
        directories.push_back(entry.path().filename().string() + "/");
      } else {
        files.push_back(entry.path().filename().string());
      }
    }
    std::sort(directories.begin(), directories.end());
    for(auto& directory : directories) {
      Populate(out, directory);
    }
    std::sort(files.begin(), files.end());
    for(auto& file : files) {
      Populate(out, file);
    }
    out << "</pre>\n<hr>\n</body>\n</html>";
    return out.str();
  }

  inline bool FileStore::IsSubdirectory(
      const std::filesystem::path& target) const {
    return std::mismatch(m_root.begin(), m_root.end(), target.begin()).first ==
      m_root.end();
  }
}

#endif
