#ifndef BEAM_READER_HPP
#define BEAM_READER_HPP
#include <concepts>
#include <limits>
#include "Beam/IO/BufferRef.hpp"
#include "Beam/IO/ValueSpan.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** Concept satisfied by types that implement the Reader interface. */
  template<typename T>
  concept IsReader = requires(
      T& t, const T& ct, std::size_t size, BufferRef buffer) {
    { ct.poll() } -> std::convertible_to<bool>;
    { t.read(out(buffer), size) } -> std::convertible_to<std::size_t>;
  };

  /**
   * Generic reader interface producing data into a Buffer.
   * @tparam B The type of Buffer to read into.
   */
  class Reader {
    public:

      /**
       * Constructs a Reader of a specified type using emplacement.
       * @tparam T The type of reader to emplace.
       * @param args The arguments to pass to the emplaced reader.
       */
      template<IsReader T, typename... Args>
      explicit Reader(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a Reader by referencing an existing reader.
       * @param reader The reader to reference.
       */
      template<DisableCopy<Reader> T> requires IsReader<dereference_t<T>>
      Reader(T&& reader);

      Reader(const Reader&) = default;

      /** Returns true if data is immediately available for reading. */
      bool poll() const;

      /**
       * Reads up to a maximum number of bytes.
       * @param destination The buffer to read into.
       * @param size The maximum number of bytes to read.
       * @return The actual number of bytes read.
       */
      template<IsBuffer B>
      std::size_t read(Out<B> destination, std::size_t size = -1);

    private:
      struct VirtualReader {
        virtual ~VirtualReader() = default;

        virtual bool poll() const = 0;
        virtual std::size_t read(BufferRef, std::size_t) = 0;
      };
      template<typename R>
      struct WrappedReader final : VirtualReader {
        using Reader = R;
        local_ptr_t<Reader> m_reader;

        template<typename... Args>
        WrappedReader(Args&&... args);

        bool poll() const override;
        std::size_t read(BufferRef, std::size_t) override;
      };
      VirtualPtr<VirtualReader> m_reader;
  };

  /**
   * Reads an exact amount of data from a Reader.
   * @param reader The Reader to read from.
   * @param buffer The Buffer to read into.
   * @param size The exact number of bytes to read.
   */
  template<IsReader R, IsBuffer B>
  void read_exact(R& reader, Out<B> buffer, std::size_t size) {
    while(size != 0) {
      auto size_read = reader.read(out(buffer), size);
      size -= size_read;
    }
  }

  template<IsReader R, typename T> requires
    std::is_trivially_copyable_v<T>&& std::is_standard_layout_v<T>
  void read(R& reader, Out<T> value) {
    auto span = ValueSpan(Ref(*value));
    reset(span);
    read_exact(reader, out(span), sizeof(T));
  }

  template<IsReader T, typename... Args>
  Reader::Reader(std::in_place_type_t<T>, Args&&... args)
    : m_reader(
        make_virtual_ptr<WrappedReader<T>>(std::forward<Args>(args)...)) {}

  template<DisableCopy<Reader> T> requires IsReader<dereference_t<T>>
  Reader::Reader(T&& reader)
    : m_reader(make_virtual_ptr<WrappedReader<std::remove_cvref_t<T>>>(
        std::forward<T>(reader))) {}

  inline bool Reader::poll() const {
    return m_reader->poll();
  }

  template<IsBuffer B>
  std::size_t Reader::read(Out<B> destination, std::size_t size) {
    return m_reader->read(*destination, size);
  }

  template<typename R>
  template<typename... Args>
  Reader::WrappedReader<R>::WrappedReader(Args&&... args)
    : m_reader(std::forward<Args>(args)...) {}

  template<typename R>
  bool Reader::WrappedReader<R>::poll() const {
    return m_reader->poll();
  }

  template<typename R>
  std::size_t Reader::WrappedReader<R>::read(BufferRef buffer,
      std::size_t size) {
    return m_reader->read(out(buffer), size);
  }
}

#endif
