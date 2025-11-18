#ifndef BEAM_BASIC_ISTREAM_READER_HPP
#define BEAM_BASIC_ISTREAM_READER_HPP
#include <algorithm>
#include <istream>
#include <type_traits>
#include <vector>
#include <boost/throw_exception.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {

  /**
   * Wraps an std::basic_istream for use with the Reader interface.
   * @tparam S The type of istream to read from.
   */
  template<typename S>
  class BasicIStreamReader {
    public:

      /**
       * The default number of bytes to request from the stream for a single
       * read.
       */
      inline static const auto DEFAULT_READ_SIZE = std::size_t(8 * 1024);

      /** The type of istream to read from. */
      using IStream = dereference_t<S>;

      /**
       * Constructs a reader that consumes bytes from the provided stream
       * source.
       * @param source The istream to read from.
       */
      template<Initializes<S> SF>
      explicit BasicIStreamReader(SF&& source);

      bool poll() const;
      template<IsBuffer R>
      std::size_t read(Out<R> destination, std::size_t size = -1);

    private:
      local_ptr_t<S> m_source;

      BasicIStreamReader(const BasicIStreamReader&) = delete;
      BasicIStreamReader& operator =(const BasicIStreamReader&) = delete;
  };

  template<typename S>
  BasicIStreamReader(S&&) -> BasicIStreamReader<std::remove_cvref_t<S>>;

  template<typename S>
  template<Initializes<S> SF>
  BasicIStreamReader<S>::BasicIStreamReader(SF&& source)
    : m_source(std::forward<SF>(source)) {}

  template<typename S>
  bool BasicIStreamReader<S>::poll() const {
    return m_source->rdbuf()->in_avail() > 0;
  }

  template<typename S>
  template<IsBuffer R>
  std::size_t BasicIStreamReader<S>::read(
      Out<R> destination, std::size_t size) {
    auto size_read = std::size_t(0);
    while(size > 0) {
      auto available_size =
        destination->grow(std::min(DEFAULT_READ_SIZE, size));
      m_source->read(get_mutable_suffix(*destination, available_size),
        static_cast<std::streamsize>(available_size));
      auto count = m_source->gcount();
      size_read += count;
      size -= count;
      destination->shrink(available_size - count);
      if(count == 0) {
        if(size_read == 0) {
          boost::throw_with_location(EndOfFileException());
        } else {
          break;
        }
      }
    }
    return size_read;
  }
}

#endif
