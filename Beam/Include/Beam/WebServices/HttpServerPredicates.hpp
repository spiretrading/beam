#ifndef BEAM_HTTPSERVERPREDICATES_HPP
#define BEAM_HTTPSERVERPREDICATES_HPP
#include "Beam/WebServices/HttpRequestSlot.hpp"
#include "Beam/WebServices/HttpServerRequest.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  //! Matches anything.
  inline HttpRequestSlot::Predicate MatchAny() {
    return [] (const HttpServerRequest& request) {
      return true;
    };
  }

  //! Tests an HTTP's method and URI path.
  /*!
    \param method The HttpMethod to match.
    \param path The URI path to match.
  */
  inline HttpRequestSlot::Predicate MatchesPath(HttpMethod method,
      const std::string& path) {
    return [=] (const HttpServerRequest& request) {
      return request.GetUri().GetPath() == path &&
        request.GetMethod() == method;
    };
  }
}
}

#endif
