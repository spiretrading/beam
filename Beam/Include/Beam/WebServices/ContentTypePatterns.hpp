#ifndef BEAM_CONTENTTYPEPATTERNS_HPP
#define BEAM_CONTENTTYPEPATTERNS_HPP
#include <filesystem>
#include <string>
#include <unordered_map>
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class ContentTypePatterns
      \brief Maps rules about file paths to content types.
   */
  class ContentTypePatterns {
    public:

      //! Returns a default set of patterns.
      static ContentTypePatterns GetDefaultPatterns();

      //! Constructs an empty set of patterns.
      ContentTypePatterns() = default;

      //! Identifies a file path's content type.
      /*!
        \param path The file path to identify.
        \return The content type for the specified <i>path</i>.
      */
      const std::string& GetContentType(
        const std::filesystem::path& path) const;

      //! Associates a content type with a file extension.
      /*!
        \param extension The extension to add.
        \param contentType The content type to associate with the
               <i>extension</i>.
      */
      void AddExtension(std::string extension, std::string contentType);

    private:
      std::string m_defaultContentType;
      std::unordered_map<std::string, std::string> m_fileExtensionContentTypes;
  };

  inline ContentTypePatterns ContentTypePatterns::GetDefaultPatterns() {
    ContentTypePatterns patterns;
    patterns.AddExtension("css", "text/css");
    patterns.AddExtension("html", "text/html");
    patterns.AddExtension("js", "application/javascript");
    patterns.AddExtension("svg", "image/svg+xml");
    return patterns;
  }

  inline const std::string& ContentTypePatterns::GetContentType(
      const std::filesystem::path& path) const {
    auto contentType = m_fileExtensionContentTypes.find(
      path.extension().string());
    if(contentType == m_fileExtensionContentTypes.end()) {
      return m_defaultContentType;
    }
    return contentType->second;
  }

  inline void ContentTypePatterns::AddExtension(std::string extension,
      std::string contentType) {
    m_fileExtensionContentTypes["." + extension] = std::move(contentType);
  }
}
}

#endif
