#ifndef AVALON_HTTPSESSION_HPP
#define AVALON_HTTPSESSION_HPP
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/noncopyable.hpp>
#include "Beam/WebServices/Cookie.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Avalon {
namespace WebServices {

  /*! \class HttpSession
      \brief Stores information about an HTTP session.
   */
  class HttpSession : private boost::noncopyable {
    public:

      //! Constructs an HttpSession.
      HttpSession();

      //! Returns the session Cookie.
      const Cookie& GetSessionCookie() const;

    private:
      Cookie m_sessionCookie;
  };
}
}

#endif // AVALON_HTTPSESSION_HPP
