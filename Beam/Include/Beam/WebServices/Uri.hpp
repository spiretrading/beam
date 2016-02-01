#ifndef BEAM_URI_HPP
#define BEAM_URI_HPP
#include <stdexcept>
#include <string>
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

      Uri(const std::string& scheme, const std::string& username,
        const std::string& password, const std::string& hostname,
        unsigned short port, const std::string& path,
        const std::string& query, const std::string& fragment);
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
}
}

#endif
