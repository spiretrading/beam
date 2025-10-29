#ifndef BEAM_HTTP_METHOD_HPP
#define BEAM_HTTP_METHOD_HPP
#include <ostream>
#include "Beam/Collections/Enum.hpp"
#ifdef DELETE
  #undef DELETE
#endif

namespace Beam {

  /** Enumerates HTTP methods. */
  BEAM_ENUM(HttpMethod,

    /** HEAD method. */
    HEAD,

    /** GET method. */
    GET,

    /** POST method. */
    POST,

    /** PUT method. */
    PUT,

    /** DELETE method. */
    DELETE,

    /** TRACE method. */
    TRACE,

    /** OPTIONS method. */
    OPTIONS,

    /** CONNECT method. */
    CONNECT,

    /** PATCH method. */
    PATCH);
}

#endif
