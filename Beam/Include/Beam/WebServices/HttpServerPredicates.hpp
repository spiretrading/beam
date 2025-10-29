#ifndef BEAM_HTTP_SERVER_PREDICATES_HPP
#define BEAM_HTTP_SERVER_PREDICATES_HPP
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpRequestSlot.hpp"

namespace Beam {

  /** Matches anything. */
  inline HttpRequestSlot::Predicate match_any() {
    return [] (const HttpRequest& request) {
      return true;
    };
  }

  /**
   * Matches anything with a specified HTTP method.
   * @param method The method to match.
   */
  inline HttpRequestSlot::Predicate match_any(HttpMethod method) {
    return [=] (const HttpRequest& request) {
      return request.get_method() == method;
    };
  }

  /**
   * Tests an HTTP's method and URI path.
   * @param method The HttpMethod to match.
   * @param path The URI path to match.
   */
  inline HttpRequestSlot::Predicate matches_path(
      HttpMethod method, const std::string& path) {
    return [=] (const HttpRequest& request) {
      return request.get_uri().get_path() == path &&
        request.get_method() == method;
    };
  }
}

#endif
