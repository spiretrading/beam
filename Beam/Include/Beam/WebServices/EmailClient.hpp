#ifndef BEAM_EMAILCLIENT_HPP
#define BEAM_EMAILCLIENT_HPP
#include <boost/noncopyable.hpp>
#include "Beam/Utilities/Concept.hpp"
#include "Beam/WebServices/Email.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \struct EmailClient.
      \brief A client used to send email messages.
   */
  struct EmailClient :  Concept<EmailClient> {
    public:

      //! Sends an email.
      /*!
        \param email The email to send.
      */
      void Send(const Email& email);

      void Open();

      void Close();
  };
}
}

#endif
