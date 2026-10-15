#ifndef BEAM_TRANSFER_ENCODING_HPP
#define BEAM_TRANSFER_ENCODING_HPP
#include <format>
#include <ostream>
#include <string_view>

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
}

template<>
class std::formatter<Beam::TransferEncoding> :
    public std::formatter<std::string_view> {
  public:
    auto format(Beam::TransferEncoding value, auto& context) const {
      auto name = [&] {
        if(value == Beam::TransferEncoding::CHUNKED) {
          return std::string_view("chunked");
        } else if(value == Beam::TransferEncoding::COMPRESS) {
          return std::string_view("compress");
        } else if(value == Beam::TransferEncoding::DEFLATE) {
          return std::string_view("deflate");
        } else if(value == Beam::TransferEncoding::GZIP) {
          return std::string_view("gzip");
        } else if(value == Beam::TransferEncoding::IDENTITY) {
          return std::string_view("identity");
        }
        return std::string_view("none");
      }();
      return std::formatter<std::string_view>::format(name, context);
    }
};

namespace Beam {
  inline std::ostream& operator <<(
      std::ostream& sink, TransferEncoding encoding) {
    return sink << std::format("{}", encoding);
  }
}

#endif
