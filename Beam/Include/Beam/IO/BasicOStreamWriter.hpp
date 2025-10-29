#ifndef BEAM_BASIC_OSTREAM_WRITER_HPP
#define BEAM_BASIC_OSTREAM_WRITER_HPP
#include <ostream>
#include <type_traits>
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Wraps an std::basic_ostream for use with the Writer interface.
   * @tparam S The type of ostream to write to.
   */
  template<typename S>
  class BasicOStreamWriter {
    public:

      /** The type of OStream being written to. */
      using OStream = dereference_t<S>;

      /**
       * Constructs a BasicOStreamWriter.
       * @param destination The basic_ostream to wrap.
       */
      template<Initializes<S> SF>
      explicit BasicOStreamWriter(SF&& destination);

      template<IsConstBuffer T>
      void write(const T& data);

    private:
      local_ptr_t<S> m_destination;

      BasicOStreamWriter(const BasicOStreamWriter&) = delete;
      BasicOStreamWriter& operator =(const BasicOStreamWriter&) = delete;
  };

  template<typename S>
  BasicOStreamWriter(S&&) -> BasicOStreamWriter<std::remove_cvref_t<S>>;

  template<typename S>
  template<Initializes<S> SF>
  BasicOStreamWriter<S>::BasicOStreamWriter(SF&& destination)
    : m_destination(std::forward<SF>(destination)) {}

  template<typename S>
  template<IsConstBuffer T>
  void BasicOStreamWriter<S>::write(const T& data) {
    m_destination->write(
      reinterpret_cast<const typename OStream::char_type*>(data.get_data()),
      data.get_size() / sizeof(typename OStream::char_type));
  }
}

#endif
