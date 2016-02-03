#ifndef BEAM_HTTPSTATUSCODE_HPP
#define BEAM_HTTPSTATUSCODE_HPP
#include <string>
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \enum HttpStatusCode
      \brief Enumerates HTTP status codes.
   */
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

  //! Returns the descriptive reason corresponding to an HttpStatusCode.
  /*!
    \param code The HttpStatusCode to get the reason for.
    \return The reason for an HttpStatusCode.
  */
  inline const std::string& GetReasonPhrase(HttpStatusCode code) {
    if(code == HttpStatusCode::CONTINUE) {
      static std::string phrase = "Continue";
      return phrase;
    } else if(code == HttpStatusCode::SWITCHING_PROTOCOLS) {
      static std::string phrase = "Switching Protocols";
      return phrase;
    } else if(code == HttpStatusCode::OK) {
      static std::string phrase = "OK";
      return phrase;
    } else if(code == HttpStatusCode::CREATED) {
      static std::string phrase = "Created";
      return phrase;
    } else if(code == HttpStatusCode::ACCEPTED) {
      static std::string phrase = "Accepted";
      return phrase;
    } else if(code == HttpStatusCode::NON_AUTHORITATIVE_INFORMATION) {
      static std::string phrase = "Non-Authoritative Information";
      return phrase;
    } else if(code == HttpStatusCode::NO_CONTENT) {
      static std::string phrase = "No Content";
      return phrase;
    } else if(code == HttpStatusCode::RESET_CONTENT) {
      static std::string phrase = "Reset Content";
      return phrase;
    } else if(code == HttpStatusCode::PARTIAL_CONTENT) {
      static std::string phrase = "Partial Content";
      return phrase;
    } else if(code == HttpStatusCode::MULTIPLE_CHOICES) {
      static std::string phrase = "Multiple Choices";
      return phrase;
    } else if(code == HttpStatusCode::MOVED_PERMANENTLY) {
      static std::string phrase = "Moved Permanently";
      return phrase;
    } else if(code == HttpStatusCode::FOUND) {
      static std::string phrase = "Found";
      return phrase;
    } else if(code == HttpStatusCode::SEE_OTHER) {
      static std::string phrase = "See Other";
      return phrase;
    } else if(code == HttpStatusCode::NOT_MODIFIED) {
      static std::string phrase = "Not Modified";
      return phrase;
    } else if(code == HttpStatusCode::USE_PROXY) {
      static std::string phrase = "Use Proxy";
      return phrase;
    } else if(code == HttpStatusCode::TEMPORARY_REDIRECT) {
      static std::string phrase = "Temporary Redirect";
      return phrase;
    } else if(code == HttpStatusCode::BAD_REQUEST) {
      static std::string phrase = "Bad Request";
      return phrase;
    } else if(code == HttpStatusCode::UNAUTHORIZED) {
      static std::string phrase = "Unauthorized";
      return phrase;
    } else if(code == HttpStatusCode::PAYMENT_REQUIRED) {
      static std::string phrase = "Payment Required";
      return phrase;
    } else if(code == HttpStatusCode::FORBIDDEN) {
      static std::string phrase = "Forbidden";
      return phrase;
    } else if(code == HttpStatusCode::NOT_FOUND) {
      static std::string phrase = "Not Found";
      return phrase;
    } else if(code == HttpStatusCode::METHOD_NOT_ALLOWED) {
      static std::string phrase = "Method Not Allowed";
      return phrase;
    } else if(code == HttpStatusCode::NOT_ACCEPTABLE) {
      static std::string phrase = "Not Acceptable";
      return phrase;
    } else if(code == HttpStatusCode::PROXY_AUTHENTICATION_REQUIRED) {
      static std::string phrase = "Proxy Authentication Required";
      return phrase;
    } else if(code == HttpStatusCode::REQUEST_TIMEOUT) {
      static std::string phrase = "Request Time-out";
      return phrase;
    } else if(code == HttpStatusCode::CONFLICT) {
      static std::string phrase = "Conflict";
      return phrase;
    } else if(code == HttpStatusCode::GONE) {
      static std::string phrase = "Gone";
      return phrase;
    } else if(code == HttpStatusCode::LENGTH_REQUIRED) {
      static std::string phrase = "Length Required";
      return phrase;
    } else if(code == HttpStatusCode::PRECONDITION_FAILED) {
      static std::string phrase = "Precondition Failed";
      return phrase;
    } else if(code == HttpStatusCode::REQUEST_ENTITY_TOO_LARGE) {
      static std::string phrase = "Request Entity Too Large";
      return phrase;
    } else if(code == HttpStatusCode::REQUEST_URI_TOO_LONG) {
      static std::string phrase = "Request-URI Too Large";
      return phrase;
    } else if(code == HttpStatusCode::UNSUPPORTED_MEDIA_TYPE) {
      static std::string phrase = "Unsupported Media Type";
      return phrase;
    } else if(code == HttpStatusCode::REQUESTED_RANGE_NOT_SATISFIABLE) {
      static std::string phrase = "Requested range not satisfiable";
      return phrase;
    } else if(code == HttpStatusCode::EXPECTATION_FAILED) {
      static std::string phrase = "Expectation Failed";
      return phrase;
    } else if(code == HttpStatusCode::INTERNAL_SERVER_ERROR) {
      static std::string phrase = "Internal Server Error";
      return phrase;
    } else if(code == HttpStatusCode::NOT_IMPLEMENTED) {
      static std::string phrase = "Not Implemented";
      return phrase;
    } else if(code == HttpStatusCode::BAD_GATEWAY) {
      static std::string phrase = "Bad Gateway";
      return phrase;
    } else if(code == HttpStatusCode::SERVICE_UNAVAILABLE) {
      static std::string phrase = "Service Unavailable";
      return phrase;
    } else if(code == HttpStatusCode::GATEWAY_TIMEOUT) {
      static std::string phrase = "Gateway Time-out";
      return phrase;
    } else if(code == HttpStatusCode::HTTP_VERSION_NOT_SUPPORTED) {
      static std::string phrase = "HTTP Version not supported";
      return phrase;
    } else {
      static std::string phrase = "Unknown";
      return phrase;
    }
  }
}
}

#endif
