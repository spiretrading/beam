#ifndef AVALON_HTTPSERVERPREDICATES_HPP
#define AVALON_HTTPSERVERPREDICATES_HPP
#include "Avalon/WebServices/HttpMethod.hpp"

namespace Avalon {
namespace WebServices {

  //! Matches anything.
  bool MatchAnyHttpRequest(const HttpServerRequest* request);

  //! Tests an HTTP's method and URI path.
  HttpRequestPredicate MatchesPath(HttpMethod method, const std::string& path);
}
}

#endif // AVALON_HTTPSERVERPREDICATES_HPP
