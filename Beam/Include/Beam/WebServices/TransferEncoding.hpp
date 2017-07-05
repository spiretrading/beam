#ifndef BEAM_TRANSFERENCODING_HPP
#define BEAM_TRANSFERENCODING_HPP
#include <ostream>
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \enum TransferEncoding
      \brief Enumerates standard transfer encodings.
   */
  enum class TransferEncoding {

    //! No encoding specified.
    NONE,

    //! Chunked encoding.
    CHUNKED,

    //! Compressed encoding.
    COMPRESS,

    //! Deflate encoding.
    DEFLATE,

    //! Gzip encoding.
    GZIP,

    //! Identity encoding.
    IDENTITY,
  };

  inline std::ostream& operator <<(std::ostream& sink,
      TransferEncoding transferEncoding) {
    if(transferEncoding == TransferEncoding::CHUNKED) {
      return (sink << "chunked");
    } else if(transferEncoding == TransferEncoding::COMPRESS) {
      return (sink << "compress");
    } else if(transferEncoding == TransferEncoding::DEFLATE) {
      return (sink << "deflate");
    } else if(transferEncoding == TransferEncoding::GZIP) {
      return (sink << "gzip");
    } else if(transferEncoding == TransferEncoding::IDENTITY) {
      return (sink << "identity");
    } else {
      return (sink << "none");
    }
  }
}
}

#endif
