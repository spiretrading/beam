#ifndef BEAM_VIRTUALWRITER_HPP
#define BEAM_VIRTUALWRITER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/IO.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /*! \class VirtualWriter
      \brief Provides a pure virtual interface to a Writer.
   */
  class VirtualWriter : private boost::noncopyable {
    public:
      using Buffer = SharedBuffer;

      virtual ~VirtualWriter() = default;

      virtual void Write(const void* data, std::size_t size) = 0;

      virtual void Write(const Buffer& data) = 0;

    protected:

      //! Constructs a VirtualWriter.
      VirtualWriter() = default;
  };

  /*! \class WrapperWriter
      \brief Wraps a Writer providing it with a virtual interface.
      \tparam WriterType The type of Writer to wrap.
   */
  template<typename WriterType>
  class WrapperWriter : public VirtualWriter {
    public:

      //! The Writer to wrap.
      using Writer = GetTryDereferenceType<WriterType>;

      using Buffer = typename Writer::Buffer;

      //! Constructs a WrapperWriter.
      /*!
        \param writer The Writer to wrap.
      */
      template<typename WriterForward>
      WrapperWriter(WriterForward&& writer);

      virtual ~WrapperWriter() override = default;

      virtual void Write(const void* data, std::size_t size) override;

      virtual void Write(const SharedBuffer& data) override;

    private:
      GetOptionalLocalPtr<WriterType> m_writer;
  };

  //! Wraps a Writer into a VirtualWriter.
  /*!
    \param writer The Writer to wrap.
  */
  template<typename Writer>
  std::unique_ptr<VirtualWriter> MakeVirtualWriter(Writer&& writer) {
    return std::make_unique<WrapperWriter<std::decay_t<Writer>>>(
      std::forward<Writer>(writer));
  }

  template<typename WriterType>
  template<typename WriterForward>
  WrapperWriter<WriterType>::WrapperWriter(WriterForward&& writer)
      : m_writer{std::forward<WriterForward>(writer)} {}

  template<typename WriterType>
  void WrapperWriter<WriterType>::Write(const void* data, std::size_t size) {
    m_writer->Write(data, size);
  }

  template<typename WriterType>
  void WrapperWriter<WriterType>::Write(const SharedBuffer& data) {
    m_writer->Write(data);
  }
}

  template<>
  struct ImplementsConcept<IO::VirtualWriter, IO::Writer<IO::SharedBuffer>> :
    std::true_type {};
}

#endif
