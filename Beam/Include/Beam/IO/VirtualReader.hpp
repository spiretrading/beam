#ifndef BEAM_VIRTUALREADER_HPP
#define BEAM_VIRTUALREADER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/IO.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /*! \class VirtualReader
      \brief Provides a pure virtual interface to a Reader.
   */
  class VirtualReader : private boost::noncopyable {
    public:
      using Buffer = SharedBuffer;

      virtual ~VirtualReader() = default;

      virtual bool IsDataAvailable() const = 0;

      virtual std::size_t Read(Out<SharedBuffer> destination) = 0;

      virtual std::size_t Read(char* destination, std::size_t size) = 0;

      virtual std::size_t Read(Out<SharedBuffer> destination,
        std::size_t size) = 0;

    protected:

      //! Constructs a VirtualReader.
      VirtualReader() = default;
  };

  /*! \class WrapperReader
      \brief Wraps a Reader providing it with a virtual interface.
      \tparam ReaderType The type of Reader to wrap.
   */
  template<typename ReaderType>
  class WrapperReader : public VirtualReader {
    public:

      //! The Reader to wrap.
      using Reader = GetTryDereferenceType<ReaderType>;

      using Buffer = typename Reader::Buffer;

      //! Constructs a WrapperReader.
      /*!
        \param reader The Reader to wrap.
      */
      template<typename ReaderForward>
      WrapperReader(ReaderForward&& reader);

      virtual ~WrapperReader() override = default;

      virtual bool IsDataAvailable() const override;

      virtual std::size_t Read(Out<SharedBuffer> destination) override;

      virtual std::size_t Read(char* destination, std::size_t size) override;

      virtual std::size_t Read(Out<SharedBuffer> destination,
        std::size_t size) override;

    private:
      GetOptionalLocalPtr<ReaderType> m_reader;
  };

  //! Wraps a Reader into a VirtualReader.
  /*!
    \param reader The Reader to wrap.
  */
  template<typename Reader>
  std::unique_ptr<VirtualReader> MakeVirtualReader(Reader&& reader) {
    return std::make_unique<WrapperReader<std::decay_t<Reader>>>(
      std::forward<Reader>(reader));
  }

  template<typename ReaderType>
  template<typename ReaderForward>
  WrapperReader<ReaderType>::WrapperReader(ReaderForward&& reader)
      : m_reader{std::forward<ReaderForward>(reader)} {}

  template<typename ReaderType>
  bool WrapperReader<ReaderType>::IsDataAvailable() const {
    return m_reader->IsDataAvailable();
  }

  template<typename ReaderType>
  std::size_t WrapperReader<ReaderType>::Read(Out<SharedBuffer> destination) {
    return m_reader->Read(Store(destination));
  }

  template<typename ReaderType>
  std::size_t WrapperReader<ReaderType>::Read(
      char* destination, std::size_t size) {
    return m_reader->Read(destination, size);
  }

  template<typename ReaderType>
  std::size_t WrapperReader<ReaderType>::Read(Out<SharedBuffer> destination,
      std::size_t size) {
    return m_reader->Read(Store(destination), size);
  }
}

  template<>
  struct ImplementsConcept<IO::VirtualReader, IO::Reader<IO::SharedBuffer>> :
    std::true_type {};
}

#endif
