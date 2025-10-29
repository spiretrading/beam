#ifndef BEAM_SIZE_DECLARATIVE_READER_HPP
#define BEAM_SIZE_DECLARATIVE_READER_HPP
#include <cstdint>
#include <limits>
#include <type_traits>
#include <boost/endian.hpp>
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {

  /**
   * Reads data whose size is declared at the beginning.
   * @tparam R The type of Reader to read from.
   */
  template<typename R> requires IsReader<dereference_t<R>>
  class SizeDeclarativeReader {
    public:

      /** The source to read from. */
      using SourceReader = dereference_t<R>;

      /**
       * Constructs a SizeDeclarativeReader.
       * @param source Used to initialize the Reader to read from.
       */
      template<Initializes<R> RF>
      explicit SizeDeclarativeReader(RF&& source);

      bool poll() const;
      template<IsBuffer B>
      std::size_t read(Out<B> destination, std::size_t size = -1);

    private:
      local_ptr_t<R> m_source;
      std::uint32_t m_read_size;
      std::uint32_t m_total_size;

      SizeDeclarativeReader(const SizeDeclarativeReader&) = delete;
      SizeDeclarativeReader& operator =(const SizeDeclarativeReader&) = delete;
      void read_size();
  };

  template<typename R>
  SizeDeclarativeReader(R&&) ->
    SizeDeclarativeReader<std::remove_cvref_t<R>>;

  template<typename R> requires IsReader<dereference_t<R>>
  template<Initializes<R> RF>
  SizeDeclarativeReader<R>::SizeDeclarativeReader(RF&& source)
    : m_source(std::forward<RF>(source)),
      m_read_size(0),
      m_total_size(0) {}

  template<typename R> requires IsReader<dereference_t<R>>
  bool SizeDeclarativeReader<R>::poll() const {
    return false;
  }

  template<typename R> requires IsReader<dereference_t<R>>
  template<IsBuffer B>
  std::size_t SizeDeclarativeReader<R>::read(
      Out<B> destination, std::size_t size) {
    if(m_read_size == m_total_size) {
      read_size();
    }
    auto offset = std::size_t(0);
    while(size > 0 && m_read_size != m_total_size) {
      auto next_read_size =
        std::min(size, static_cast<std::size_t>(m_total_size - m_read_size));
      try {
        auto actual_read_size =
          m_source->read(out(destination), next_read_size);
        offset += actual_read_size;
        m_read_size += actual_read_size;
        size -= actual_read_size;
      } catch(const std::exception&) {
        m_read_size = 0;
        m_total_size = 0;
        throw;
      }
    }
    return offset;
  }

  template<typename R> requires IsReader<dereference_t<R>>
  void SizeDeclarativeReader<R>::read_size() {
    try {
      Beam::read(*m_source, out(m_total_size));
    } catch(const std::exception&) {
      m_read_size = 0;
      m_total_size = 0;
      throw;
    }
    m_total_size = boost::endian::little_to_native(m_total_size);
    m_read_size = 0;
  }
}

#endif
