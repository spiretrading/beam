#ifndef BEAM_HTTPSERVERPREDICATES_HPP
#define BEAM_HTTPSERVERPREDICATES_HPP
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpRequestSlot.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  //! Matches anything.
  inline HttpRequestSlot::Predicate MatchAny() {
    return [] (const HttpRequest& request) {
      return true;
    };
  }

  //! Matches anything with a specified HTTP method.
  /*!
    \param method The method to match.
  */
  inline HttpRequestSlot::Predicate MatchAny(HttpMethod method) {
    return [=] (const HttpRequest& request) {
      return request.GetMethod() == method;
    };
  }

  //! Tests an HTTP's method and URI path.
  /*!
    \param method The HttpMethod to match.
    \param path The URI path to match.
  */
  inline HttpRequestSlot::Predicate MatchesPath(HttpMethod method,
      const std::string& path) {
    return [=] (const HttpRequest& request) {
      return request.GetUri().GetPath() == path &&
        request.GetMethod() == method;
    };
  }
}
}

#endif
