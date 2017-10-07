#ifndef BEAM_URI_HPP
#define BEAM_URI_HPP
#include <ostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <boost/exception/exception.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class Uri
      \brief Represents a uniform resource identifier.
   */
  class Uri {
    public:

      //! Constructs an empty URI.
      Uri();

      //! Constructs a URI from a string.
      /*!
        \param source The string to parse the URI from.
      */
      Uri(const std::string& source);

      //! Constructs a URI from a sequence.
      /*!
        \param first The character to parse.
        \param last One past the last character to parse.
      */
      Uri(const char* first, const char* last);

      //! Returns the scheme.
      const std::string& GetScheme() const;

      //! Returns the username.
      const std::string& GetUsername() const;

      //! Returns the password.
      const std::string& GetPassword() const;

      //! Returns the hostname.
      const std::string& GetHostname() const;

      //! Returns the port.
      unsigned short GetPort() const;

      //! Sets the port.
      void SetPort(unsigned short port);

      //! Returns the path.
      const std::string& GetPath() const;

      //! Returns the query.
      const std::string& GetQuery() const;

      //! Returns the fragment.
      const std::string& GetFragment() const;

    private:
      std::string m_scheme;
      std::string m_username;
      std::string m_password;
      std::string m_hostname;
      unsigned short m_port;
      std::string m_path;
      std::string m_query;
      std::string m_fragment;
  };

  /*! \class MalformedUriException
      \brief Signals an invalid/malformed URI.
   */
  class MalformedUriException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a MalformedUriException.
      MalformedUriException();

      //! Constructs a MalformedUriException.
      /*!
        \param message A message describing the error.
      */
      MalformedUriException(const std::string& message);

      virtual ~MalformedUriException() throw();
  };

  inline std::ostream& operator <<(std::ostream& sink, const Uri& uri) {
    if(!uri.GetScheme().empty()) {
      sink << uri.GetScheme() << ':';
    }
    if(!uri.GetHostname().empty()) {
      sink << '/' << '/';
    }
    if(!uri.GetUsername().empty()) {
      sink << uri.GetUsername() << ':' << uri.GetPassword() << '@';
    } else if(!uri.GetPassword().empty()) {
      sink << uri.GetUsername() << ':' << uri.GetPassword() << '@';
    }
    sink << uri.GetHostname();
    if(uri.GetPort() != 0) {
      sink << ':' << uri.GetPort();
    }
    sink << uri.GetPath();
    if(!uri.GetQuery().empty()) {
      sink << '?' << uri.GetQuery();
    }
    if(!uri.GetFragment().empty()) {
      sink << '#' << uri.GetFragment();
    }
    return sink;
  }

  inline Uri::Uri()
      : m_port{0} {}

  inline Uri::Uri(const std::string& source)
      : Uri{source.c_str(), source.c_str() + source.size()} {}

  inline Uri::Uri(const char* first, const char* last) {
    static const std::regex URI_PATTERN{
      "^((\\w+):)?(\\/\\/(((\\w*):(\\w*)@)?"
      "([^\\/\\?:]*)(:(\\d+))?))?(\\/?([^\\/\\?#][^\\?#]*)"
      "?)?(\\?([^#]+))?(#(\\w*))?"};
    const auto SCHEME_CAPTURE = 2;
    const auto USERNAME_CAPTURE = 6;
    const auto PASSWORD_CAPTURE = 7;
    const auto HOSTNAME_CAPTURE = 8;
    const auto PORT_CAPTURE = 10;
    const auto PATHNAME_CAPTURE = 11;
    const auto QUERY_CAPTURE = 14;
    const auto FRAGMENT_CAPTURE = 16;
    std::cmatch matcher;
    if(!std::regex_match(first, last, matcher, URI_PATTERN)) {
      BOOST_THROW_EXCEPTION(MalformedUriException{"URI is not well formed."});
    }
    if(matcher[PORT_CAPTURE].str().empty()) {
      m_port = 0;
    } else {
      auto port = std::stoul(matcher[PORT_CAPTURE].str());
      if(port > std::numeric_limits<unsigned short>::max()) {
        BOOST_THROW_EXCEPTION(MalformedUriException{"URI is not well formed."});
      } else {
        m_port = static_cast<unsigned short>(port);
      }
    }
    m_scheme = matcher[SCHEME_CAPTURE];
    m_username = matcher[USERNAME_CAPTURE];
    m_password = matcher[PASSWORD_CAPTURE];
    m_hostname = matcher[HOSTNAME_CAPTURE];
    m_path = matcher[PATHNAME_CAPTURE];
    m_query = matcher[QUERY_CAPTURE];
    m_fragment = matcher[FRAGMENT_CAPTURE];;
  }

  inline const std::string& Uri::GetScheme() const {
    return m_scheme;
  }

  inline const std::string& Uri::GetUsername() const {
    return m_username;
  }

  inline const std::string& Uri::GetPassword() const {
    return m_password;
  }

  inline const std::string& Uri::GetHostname() const {
    return m_hostname;
  }

  inline unsigned short Uri::GetPort() const {
    if(m_port == 0) {
      if(GetScheme() == "http" || GetScheme() == "ws") {
        return static_cast<std::uint16_t>(80);
      } else if(GetScheme() == "https" || GetScheme() == "wss") {
        return static_cast<std::uint16_t>(443);
      }
    }
    return m_port;
  }

  inline void Uri::SetPort(unsigned short port) {
    m_port = port;
  }

  inline const std::string& Uri::GetPath() const {
    return m_path;
  }

  inline const std::string& Uri::GetQuery() const {
    return m_query;
  }

  inline const std::string& Uri::GetFragment() const {
    return m_fragment;
  }

  inline MalformedUriException::MalformedUriException()
      : std::runtime_error{"Malformed URI."} {}

  inline MalformedUriException::MalformedUriException(
      const std::string& message)
      : std::runtime_error{message} {}

  inline MalformedUriException::~MalformedUriException() throw() {}
}
}

#endif
