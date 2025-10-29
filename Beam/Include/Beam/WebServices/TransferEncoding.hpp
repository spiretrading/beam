#ifndef BEAM_TRANSFER_ENCODING_HPP
#define BEAM_TRANSFER_ENCODING_HPP
#include <ostream>

namespace Beam {

  /** Enumerates standard transfer encodings. */
  enum class TransferEncoding {

    /** No encoding specified. */
    NONE,

    /** Chunked encoding. */
    CHUNKED,

    /** Compressed encoding. */
    COMPRESS,

    /** Deflate encoding. */
    DEFLATE,

    /** Gzip encoding. */
    GZIP,

    /** Identity encoding. */
    IDENTITY
  };

  inline std::ostream& operator <<(
      std::ostream& sink, TransferEncoding encoding) {
    if(encoding == TransferEncoding::CHUNKED) {
      return (sink << "chunked");
    } else if(encoding == TransferEncoding::COMPRESS) {
      return (sink << "compress");
    } else if(encoding == TransferEncoding::DEFLATE) {
      return (sink << "deflate");
    } else if(encoding == TransferEncoding::GZIP) {
      return (sink << "gzip");
    } else if(encoding == TransferEncoding::IDENTITY) {
      return (sink << "identity");
    } else {
      return (sink << "none");
    }
  }
}

#endif
