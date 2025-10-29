#ifndef BEAM_NULL_READER_HPP
#define BEAM_NULL_READER_HPP
#include <boost/throw_exception.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/Reader.hpp"

namespace Beam {

  /** A Reader that contains no data. */
  class NullReader {
    public:
      bool poll() const;
      template<IsBuffer R>
      std::size_t read(Out<R> destination, std::size_t size = -1);
  };

  inline bool NullReader::poll() const {
    return false;
  }

  template<IsBuffer R>
  std::size_t NullReader::read(Out<R> destination, std::size_t size) {
    boost::throw_with_location(EndOfFileException());
  }
}

#endif
