#ifndef BEAM_HTTP_STATUS_CODE_HPP
#define BEAM_HTTP_STATUS_CODE_HPP
#include <string>

namespace Beam {

  /** Enumerates HTTP status codes. */
  enum class HttpStatusCode {
    CONTINUE = 100,
    SWITCHING_PROTOCOLS = 101,
    PROCESSING = 102,
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NON_AUTHORITATIVE_INFORMATION = 203,
    NO_CONTENT = 204,
    RESET_CONTENT = 205,
    PARTIAL_CONTENT = 206,
    MULTI_STATUS = 207,
    MULTIPLE_CHOICES = 300,
    MOVED_PERMANENTLY = 301,
    FOUND = 302,
    SEE_OTHER = 303,
    NOT_MODIFIED = 304,
    USE_PROXY = 305,
    SWITCH_PROXY = 306,
    TEMPORARY_REDIRECT = 307,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    PAYMENT_REQUIRED = 402,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    NOT_ACCEPTABLE = 406,
    PROXY_AUTHENTICATION_REQUIRED = 407,
    REQUEST_TIMEOUT = 408,
    CONFLICT = 409,
    GONE = 410,
    LENGTH_REQUIRED = 411,
    PRECONDITION_FAILED = 412,
    REQUEST_ENTITY_TOO_LARGE = 413,
    REQUEST_URI_TOO_LONG = 414,
    UNSUPPORTED_MEDIA_TYPE = 415,
    REQUESTED_RANGE_NOT_SATISFIABLE = 416,
    EXPECTATION_FAILED = 417,
    IM_A_TEAPOT = 418,
    UNPROCESSABLE_ENTITY = 422,
    LOCKED = 423,
    FAILED_DEPENDENCY = 424,
    UNORDERED_COLLECTION = 425,
    UPGRADE_REQUIRED = 426,
    NO_RESPONSE = 444,
    RETRY_WITH = 449,
    BLOCK_BY_WINDOWS_PARENTAL_CONTROLS = 450,
    CLIENT_CLOSED_REQUEST = 499,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    BAD_GATEWAY = 502,
    SERVICE_UNAVAILABLE = 503,
    GATEWAY_TIMEOUT = 504,
    HTTP_VERSION_NOT_SUPPORTED = 505,
    VARIANT_ALSO_NEGOTIATES = 506,
    INSUFFICIENT_STORAGE = 507,
    BANDWIDTH_LIMIT_EXCEEDED = 509,
    NOT_EXTENDED = 510
  };

  /**
   * Returns the descriptive reason corresponding to an HttpStatusCode.
   * @param code The HttpStatusCode to get the reason for.
   * @return The reason for an HttpStatusCode.
   */
  inline const std::string& get_reason_phrase(HttpStatusCode code) {
    if(code == HttpStatusCode::CONTINUE) {
      static const auto phrase = std::string("Continue");
      return phrase;
    } else if(code == HttpStatusCode::SWITCHING_PROTOCOLS) {
      static const auto phrase = std::string("Switching Protocols");
      return phrase;
    } else if(code == HttpStatusCode::OK) {
      static const auto phrase = std::string("OK");
      return phrase;
    } else if(code == HttpStatusCode::CREATED) {
      static const auto phrase = std::string("Created");
      return phrase;
    } else if(code == HttpStatusCode::ACCEPTED) {
      static const auto phrase = std::string("Accepted");
      return phrase;
    } else if(code == HttpStatusCode::NON_AUTHORITATIVE_INFORMATION) {
      static const auto phrase =
        std::string("Non-Authoritative Information");
      return phrase;
    } else if(code == HttpStatusCode::NO_CONTENT) {
      static const auto phrase = std::string("No Content");
      return phrase;
    } else if(code == HttpStatusCode::RESET_CONTENT) {
      static const auto phrase = std::string("Reset Content");
      return phrase;
    } else if(code == HttpStatusCode::PARTIAL_CONTENT) {
      static const auto phrase = std::string("Partial Content");
      return phrase;
    } else if(code == HttpStatusCode::MULTIPLE_CHOICES) {
      static const auto phrase = std::string("Multiple Choices");
      return phrase;
    } else if(code == HttpStatusCode::MOVED_PERMANENTLY) {
      static const auto phrase = std::string("Moved Permanently");
      return phrase;
    } else if(code == HttpStatusCode::FOUND) {
      static const auto phrase = std::string("Found");
      return phrase;
    } else if(code == HttpStatusCode::SEE_OTHER) {
      static const auto phrase = std::string("See Other");
      return phrase;
    } else if(code == HttpStatusCode::NOT_MODIFIED) {
      static const auto phrase = std::string("Not Modified");
      return phrase;
    } else if(code == HttpStatusCode::USE_PROXY) {
      static const auto phrase = std::string("Use Proxy");
      return phrase;
    } else if(code == HttpStatusCode::TEMPORARY_REDIRECT) {
      static const auto phrase = std::string("Temporary Redirect");
      return phrase;
    } else if(code == HttpStatusCode::BAD_REQUEST) {
      static const auto phrase = std::string("Bad Request");
      return phrase;
    } else if(code == HttpStatusCode::UNAUTHORIZED) {
      static const auto phrase = std::string("Unauthorized");
      return phrase;
    } else if(code == HttpStatusCode::PAYMENT_REQUIRED) {
      static const auto phrase = std::string("Payment Required");
      return phrase;
    } else if(code == HttpStatusCode::FORBIDDEN) {
      static const auto phrase = std::string("Forbidden");
      return phrase;
    } else if(code == HttpStatusCode::NOT_FOUND) {
      static const auto phrase = std::string("Not Found");
      return phrase;
    } else if(code == HttpStatusCode::METHOD_NOT_ALLOWED) {
      static const auto phrase = std::string("Method Not Allowed");
      return phrase;
    } else if(code == HttpStatusCode::NOT_ACCEPTABLE) {
      static const auto phrase = std::string("Not Acceptable");
      return phrase;
    } else if(code == HttpStatusCode::PROXY_AUTHENTICATION_REQUIRED) {
      static const auto phrase =
        std::string("Proxy Authentication Required");
      return phrase;
    } else if(code == HttpStatusCode::REQUEST_TIMEOUT) {
      static const auto phrase = std::string("Request Time-out");
      return phrase;
    } else if(code == HttpStatusCode::CONFLICT) {
      static const auto phrase = std::string("Conflict");
      return phrase;
    } else if(code == HttpStatusCode::GONE) {
      static const auto phrase = std::string("Gone");
      return phrase;
    } else if(code == HttpStatusCode::LENGTH_REQUIRED) {
      static const auto phrase = std::string("Length Required");
      return phrase;
    } else if(code == HttpStatusCode::PRECONDITION_FAILED) {
      static const auto phrase = std::string("Precondition Failed");
      return phrase;
    } else if(code == HttpStatusCode::REQUEST_ENTITY_TOO_LARGE) {
      static const auto phrase = std::string("Request Entity Too Large");
      return phrase;
    } else if(code == HttpStatusCode::REQUEST_URI_TOO_LONG) {
      static const auto phrase = std::string("Request-URI Too Large");
      return phrase;
    } else if(code == HttpStatusCode::UNSUPPORTED_MEDIA_TYPE) {
      static const auto phrase = std::string("Unsupported Media Type");
      return phrase;
    } else if(code == HttpStatusCode::REQUESTED_RANGE_NOT_SATISFIABLE) {
      static const auto phrase =
        std::string("Requested range not satisfiable");
      return phrase;
    } else if(code == HttpStatusCode::EXPECTATION_FAILED) {
      static const auto phrase = std::string("Expectation Failed");
      return phrase;
    } else if(code == HttpStatusCode::INTERNAL_SERVER_ERROR) {
      static const auto phrase = std::string("Internal Server Error");
      return phrase;
    } else if(code == HttpStatusCode::NOT_IMPLEMENTED) {
      static const auto phrase = std::string("Not Implemented");
      return phrase;
    } else if(code == HttpStatusCode::BAD_GATEWAY) {
      static const auto phrase = std::string("Bad Gateway");
      return phrase;
    } else if(code == HttpStatusCode::SERVICE_UNAVAILABLE) {
      static const auto phrase = std::string("Service Unavailable");
      return phrase;
    } else if(code == HttpStatusCode::GATEWAY_TIMEOUT) {
      static const auto phrase = std::string("Gateway Time-out");
      return phrase;
    } else if(code == HttpStatusCode::HTTP_VERSION_NOT_SUPPORTED) {
      static const auto phrase = std::string("HTTP Version not supported");
      return phrase;
    } else {
      static const auto phrase = std::string("Unknown");
      return phrase;
    }
  }
}

#endif
