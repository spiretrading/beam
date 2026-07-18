module;
#include "Prelude.hpp"

export module Beam:HttpMethod;

#ifdef DELETE
  #undef DELETE
#endif

export namespace Beam {

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

