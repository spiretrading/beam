#ifndef BEAM_SIZE_DECLARATIVE_WRITER_HPP
#define BEAM_SIZE_DECLARATIVE_WRITER_HPP
#include <type_traits>
#include <boost/endian.hpp>
#include "Beam/IO/IOException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Writes to a destination, declaring the size to be written.
   * @tparam W The Writer to write to.
   */
  template<typename W> requires IsWriter<dereference_t<W>>
  class SizeDeclarativeWriter {
    public:

      /** The destination to write to. */
      using DestinationWriter = dereference_t<W>;

      /**
       * Constructs a SizeDeclarativeWriter.
       * @param destination Used to initialize the destination of all writes.
       */
      template<Initializes<W> WF>
      explicit SizeDeclarativeWriter(WF&& destination);

      template<IsConstBuffer T>
      void write(const T& data);

    private:
      local_ptr_t<W> m_destination;

      SizeDeclarativeWriter(const SizeDeclarativeWriter&) = delete;
      SizeDeclarativeWriter& operator =(const SizeDeclarativeWriter&) = delete;
  };

  template<typename W>
  SizeDeclarativeWriter(W&&) ->
    SizeDeclarativeWriter<std::remove_cvref_t<W>>;

  template<typename W> requires IsWriter<dereference_t<W>>
  template<Initializes<W> WF>
  SizeDeclarativeWriter<W>::SizeDeclarativeWriter(WF&& destination)
    : m_destination(std::forward<WF>(destination)) {}

  template<typename W> requires IsWriter<dereference_t<W>>
  template<IsConstBuffer T>
  void SizeDeclarativeWriter<W>::write(const T& data) {
    if(data.get_size() == 0) {
      return;
    }
    auto buffer = SharedBuffer();
    try {
      append(buffer, boost::endian::native_to_little(
        static_cast<std::uint32_t>(data.get_size())));
      append(buffer, data);
    } catch(const std::exception&) {
      std::throw_with_nested(IOException());
    }
    m_destination->write(std::move(buffer));
  }
}

#endif
