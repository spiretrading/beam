#ifndef BEAM_VIRTUAL_READER_HPP
#define BEAM_VIRTUAL_READER_HPP
#include "Beam/IO/IO.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /** Provides a pure virtual interface to a Reader. */
  class VirtualReader {
    public:
      using Buffer = SharedBuffer;

      virtual ~VirtualReader() = default;

      virtual bool IsDataAvailable() const = 0;

      virtual std::size_t Read(Out<SharedBuffer> destination) = 0;

      virtual std::size_t Read(char* destination, std::size_t size) = 0;

      virtual std::size_t Read(Out<SharedBuffer> destination,
        std::size_t size) = 0;

    protected:

      /** Constructs a VirtualReader. */
      VirtualReader() = default;

    private:
      VirtualReader(const VirtualReader&) = delete;
      VirtualReader& operator =(const VirtualReader&) = delete;
  };

  /**
   * Wraps a Reader providing it with a virtual interface.
   * @param <R> The type of Reader to wrap.
   */
  template<typename R>
  class WrapperReader : public VirtualReader {
    public:

      /** The Reader to wrap. */
      using Reader = GetTryDereferenceType<R>;

      using Buffer = typename Reader::Buffer;

      /**
       * Constructs a WrapperReader.
       * @param reader The Reader to wrap.
       */
      template<typename RF>
      WrapperReader(RF&& reader);

      bool IsDataAvailable() const override;

      std::size_t Read(Out<SharedBuffer> destination) override;

      std::size_t Read(char* destination, std::size_t size) override;

      std::size_t Read(Out<SharedBuffer> destination,
        std::size_t size) override;

    private:
      GetOptionalLocalPtr<R> m_reader;
  };

  /**
   * Wraps a Reader into a VirtualReader.
   * @param reader The Reader to wrap.
   */
  template<typename Reader>
  std::unique_ptr<VirtualReader> MakeVirtualReader(Reader&& reader) {
    return std::make_unique<WrapperReader<std::decay_t<Reader>>>(
      std::forward<Reader>(reader));
  }

  template<typename R>
  template<typename RF>
  WrapperReader<R>::WrapperReader(RF&& reader)
    : m_reader(std::forward<RF>(reader)) {}

  template<typename R>
  bool WrapperReader<R>::IsDataAvailable() const {
    return m_reader->IsDataAvailable();
  }

  template<typename R>
  std::size_t WrapperReader<R>::Read(Out<SharedBuffer> destination) {
    return m_reader->Read(Store(destination));
  }

  template<typename R>
  std::size_t WrapperReader<R>::Read(char* destination, std::size_t size) {
    return m_reader->Read(destination, size);
  }

  template<typename R>
  std::size_t WrapperReader<R>::Read(Out<SharedBuffer> destination,
      std::size_t size) {
    return m_reader->Read(Store(destination), size);
  }
}

  template<>
  struct ImplementsConcept<IO::VirtualReader, IO::Reader<IO::SharedBuffer>> :
    std::true_type {};
}

#endif
