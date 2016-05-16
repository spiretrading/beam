#ifndef BEAM_HTTPHEADER_HPP
#define BEAM_HTTPHEADER_HPP
#include <ostream>
#include <string>
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class HttpHeader
      \brief Stores an HTTP header.
   */
  class HttpHeader {
    public:

      //! Constructs an HttpHeader.
      /*!
        \param name The header's name.
        \param value The header's value.
      */
      HttpHeader(std::string name, std::string value);

      //! Returns the header's name.
      const std::string& GetName() const;

      //! Returns the header's value.
      const std::string& GetValue() const;

    private:
      std::string m_name;
      std::string m_value;
  };

  inline std::ostream& operator <<(std::ostream& sink,
      const HttpHeader& header) {
    return sink << header.GetName() << ':' << ' ' << header.GetValue();
  }

  inline HttpHeader::HttpHeader(std::string name, std::string value)
      : m_name{std::move(name)},
        m_value{std::move(value)} {}

  inline const std::string& HttpHeader::GetName() const {
    return m_name;
  }

  inline const std::string& HttpHeader::GetValue() const {
    return m_value;
  }
}
}

#endif
