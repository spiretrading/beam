#ifndef BEAM_FILE_STORE_HPP
#define BEAM_FILE_STORE_HPP
#include <filesystem>
#include <fstream>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/WebServices/ContentTypePatterns.hpp"
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpRequestSlot.hpp"
#include "Beam/WebServices/HttpResponse.hpp"

namespace Beam {

  /** Handles an HTTP request to serve a file. */
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
       * @param patterns The set of patterns to use for content
       *        types.
       */
      FileStore(std::filesystem::path root, ContentTypePatterns patterns);

      /**
       * Serves a file from a specified path.
       * @param path The path to the file to serve.
       * @return The HTTP response containing the file contents.
       */
      HttpResponse serve(const std::filesystem::path& path);

      /**
       * Serves a file from an HTTP request.
       * @param request The HTTP request to serve.
       * @return The HTTP response containing the file contents.
       */
      HttpResponse serve(const HttpRequest& request);

      /**
       * Serves a file from a specified path.
       * @param path The path to the file to serve.
       * @param response Stores the HTTP response containing the file contents.
       */
      void serve(const std::filesystem::path& path, Out<HttpResponse> response);

      /**
       * Serves a file from an HTTP request.
       * @param request The HTTP request to serve.
       * @param response Stores the HTTP response containing the file contents.
       */
      void serve(const HttpRequest& request, Out<HttpResponse> response);

    private:
      std::filesystem::path m_root;
      ContentTypePatterns m_patterns;

      FileStore(const FileStore&) = delete;
      FileStore& operator =(const FileStore&) = delete;
      static void populate(std::ostream& out, const std::string& name);
      static std::string make_display_path(const std::filesystem::path& path);
      std::string make_directory_listing(
        const std::filesystem::path& target) const;
      bool is_subdirectory(const std::filesystem::path& target) const;
  };

  /**
   * Returns an HttpRequestSlot to serve index.html.
   * @param file_store The FileStore serving the index.html file.
   * @return An HttpRequestSlot that serves index.html.
   */
  inline HttpRequestSlot serve_index(FileStore& file_store) {
    return {
      [] (const HttpRequest& request) {
        return (request.get_uri().get_path() == "/" ||
          request.get_uri().get_path().empty()) &&
            request.get_method() == HttpMethod::GET;
      },
      [&] (const HttpRequest& request) {
        auto response = HttpResponse();
        file_store.serve("index.html", out(response));
        return response;
      }
    };
  }

  inline FileStore::FileStore(std::filesystem::path root)
    : FileStore(std::move(root), ContentTypePatterns::get_default_patterns()) {}

  inline FileStore::FileStore(
    std::filesystem::path root, ContentTypePatterns patterns)
    : m_root(std::filesystem::canonical(std::filesystem::absolute(root))),
      m_patterns(std::move(patterns)) {}

  inline HttpResponse FileStore::serve(const std::filesystem::path& path) {
    auto response = HttpResponse();
    serve(path, out(response));
    return response;
  }

  inline HttpResponse FileStore::serve(const HttpRequest& request) {
    auto path = request.get_uri().get_path();
    if(!path.empty() && path[0] == '/') {
      return serve(path.substr(1));
    } else {
      return serve(path);
    }
  }

  inline void FileStore::serve(
      const std::filesystem::path& path, Out<HttpResponse> response) {
    auto error_code = std::error_code();
    auto full_path = std::filesystem::canonical(m_root / path, error_code);
    if(error_code || !is_subdirectory(full_path)) {
      response->set_status_code(HttpStatusCode::NOT_FOUND);
      return;
    }
    auto buffer = SharedBuffer();
    if(std::filesystem::is_directory(full_path)) {
      auto listing = make_directory_listing(full_path);
      append(buffer, listing.c_str(), listing.size());
      response->set_header(HttpHeader("Content-Type", "text/html"));
    } else {
      auto file = std::ifstream(full_path, std::ios::in | std::ios::binary);
      if(!file) {
        response->set_status_code(HttpStatusCode::NOT_FOUND);
        return;
      }
      buffer.grow(
        static_cast<std::size_t>(std::filesystem::file_size(full_path)));
      file.read(buffer.get_mutable_data(), buffer.get_size());
      auto& content_type = m_patterns.get_content_type(full_path);
      if(!content_type.empty()) {
        response->set_header(HttpHeader("Content-Type", content_type));
      }
    }
    response->set_body(buffer);
  }

  inline void FileStore::serve(
      const HttpRequest& request, Out<HttpResponse> response) {
    serve(request.get_uri().get_path(), out(response));
  }

  inline void FileStore::populate(std::ostream& out, const std::string& name) {
    out << "<a href=\"" << name << "\">" << name << "</a>\n";
  }

  inline std::string FileStore::make_display_path(
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

  inline std::string FileStore::make_directory_listing(
      const std::filesystem::path& target) const {
    auto relative_path = std::filesystem::relative(target, m_root);
    auto display_path = [&] {
      if(target == m_root) {
        return std::string("/");
      } else {
        return "/" + make_display_path(relative_path);
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
      populate(out, directory);
    }
    std::sort(files.begin(), files.end());
    for(auto& file : files) {
      populate(out, file);
    }
    out << "</pre>\n<hr>\n</body>\n</html>";
    return out.str();
  }

  inline bool FileStore::is_subdirectory(
      const std::filesystem::path& target) const {
    return std::mismatch(m_root.begin(), m_root.end(), target.begin()).first ==
      m_root.end();
  }
}

#endif
