#ifndef BEAM_HTTPMETHOD_HPP
#define BEAM_HTTPMETHOD_HPP
#include <ostream>
#include "Beam/Collections/Enum.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \enum HttpMethod
      \brief Enumerates HTTP methods.
   */
  BEAM_ENUM(HttpMethod,

    //! HEAD method.
    HEAD,

    //! GET method.
    GET,

    //! POST method.
    POST,

    //! PUT method.
    PUT,

    //! DELETE method.
    DELETE,

    //! TRACE method.
    TRACE,

    //! OPTIONS method.
    OPTIONS,

    //! CONNECT method.
    CONNECT,

    //! PATCH method.
    PATCH);

  inline std::ostream& operator <<(std::ostream& sink, HttpMethod method) {
    if(method == HttpMethod::HEAD) {
      return (sink << "HEAD");
    } else if(method == HttpMethod::GET) {
      return (sink << "GET");
    } else if(method == HttpMethod::POST) {
      return (sink << "POST");
    } else if(method == HttpMethod::PUT) {
      return (sink << "PUT");
    } else if(method == HttpMethod::DELETE) {
      return (sink << "DELETE");
    } else if(method == HttpMethod::TRACE) {
      return (sink << "TRACE");
    } else if(method == HttpMethod::OPTIONS) {
      return (sink << "OPTIONS");
    } else if(method == HttpMethod::CONNECT) {
      return (sink << "CONNECT");
    } else if(method == HttpMethod::PATCH) {
      return (sink << "PATCH");
    } else {
      return (sink << "NONE");
    }
  }
}
}

#endif
