#ifndef BEAM_CONTENT_TYPE_PATTERNS_HPP
#define BEAM_CONTENT_TYPE_PATTERNS_HPP
#include <filesystem>
#include <string>
#include <unordered_map>

namespace Beam {

  /** Maps rules about file paths to content types. */
  class ContentTypePatterns {
    public:

      /** Returns a default set of patterns. */
      static ContentTypePatterns get_default_patterns();

      /** Constructs an empty set of patterns. */
      ContentTypePatterns() = default;

      /**
       * Identifies a file path's content type.
       * @param path The file path to identify.
       * @return The content type for the specified path.
       */
      const std::string& get_content_type(
        const std::filesystem::path& path) const;

      /**
       * Associates a content type with a file extension.
       * @param extension The extension to add.
       * @param content_type The content type to associate with the extension.
       */
      void add_extension(std::string extension, std::string content_type);

    private:
      std::string m_default_content_type;
      std::unordered_map<std::string, std::string>
        m_file_extension_content_types;
  };

  inline ContentTypePatterns ContentTypePatterns::get_default_patterns() {
    auto patterns = ContentTypePatterns();
    patterns.add_extension("css", "text/css");
    patterns.add_extension("html", "text/html");
    patterns.add_extension("js", "application/javascript");
    patterns.add_extension("svg", "image/svg+xml");
    return patterns;
  }

  inline const std::string& ContentTypePatterns::get_content_type(
      const std::filesystem::path& path) const {
    auto content_type =
      m_file_extension_content_types.find(path.extension().string());
    if(content_type == m_file_extension_content_types.end()) {
      return m_default_content_type;
    }
    return content_type->second;
  }

  inline void ContentTypePatterns::add_extension(
      std::string extension, std::string content_type) {
    m_file_extension_content_types["." + extension] = std::move(content_type);
  }
}

#endif
