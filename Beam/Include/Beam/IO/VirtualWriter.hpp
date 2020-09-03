#ifndef BEAM_VIRTUAL_WRITER_HPP
#define BEAM_VIRTUAL_WRITER_HPP
#include "Beam/IO/IO.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /** Provides a pure virtual interface to a Writer. */
  class VirtualWriter {
    public:
      using Buffer = SharedBuffer;

      virtual ~VirtualWriter() = default;

      virtual void Write(const void* data, std::size_t size) = 0;

      virtual void Write(const Buffer& data) = 0;

    protected:

      /** Constructs a VirtualWriter. */
      VirtualWriter() = default;

    private:
      VirtualWriter(const VirtualWriter&) = delete;
      VirtualWriter& operator =(const VirtualWriter&) = delete;
  };

  /**
   * Wraps a Writer providing it with a virtual interface.
   * @param <W> The type of Writer to wrap.
   */
  template<typename W>
  class WrapperWriter : public VirtualWriter {
    public:

      /** The Writer to wrap. */
      using Writer = GetTryDereferenceType<W>;
      using Buffer = typename Writer::Buffer;

      /**
       * Constructs a WrapperWriter.
       * @param writer The Writer to wrap.
       */
      template<typename WF>
      WrapperWriter(WF&& writer);

      void Write(const void* data, std::size_t size) override;

      void Write(const SharedBuffer& data) override;

    private:
      GetOptionalLocalPtr<W> m_writer;
  };

  /**
   * Wraps a Writer into a VirtualWriter.
   * @param writer The Writer to wrap.
   */
  template<typename Writer>
  std::unique_ptr<VirtualWriter> MakeVirtualWriter(Writer&& writer) {
    return std::make_unique<WrapperWriter<std::decay_t<Writer>>>(
      std::forward<Writer>(writer));
  }

  template<typename W>
  template<typename WF>
  WrapperWriter<W>::WrapperWriter(WF&& writer)
    : m_writer(std::forward<WF>(writer)) {}

  template<typename W>
  void WrapperWriter<W>::Write(const void* data, std::size_t size) {
    m_writer->Write(data, size);
  }

  template<typename W>
  void WrapperWriter<W>::Write(const SharedBuffer& data) {
    m_writer->Write(data);
  }
}

  template<>
  struct ImplementsConcept<IO::VirtualWriter, IO::Writer<IO::SharedBuffer>> :
    std::true_type {};
}

#endif
