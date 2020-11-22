#ifndef BEAM_WRITER_BOX_HPP
#define BEAM_WRITER_BOX_HPP
#include <memory>
#include <type_traits>
#include "Beam/IO/BufferView.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /** Provides a generic interface over an arbitrary Writer. */
  class WriterBox {
    public:
      using Buffer = SharedBuffer;

      /**
       * Constructs a WriterBox of a specified type using emplacement.
       * @param <T> The type of writer to emplace.
       * @param args The arguments to pass to the emplaced writer.
       */
      template<typename T, typename... Args>
      explicit WriterBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a WriterBox by copying an existing writer.
       * @param writer The writer to copy.
       */
      template<typename Writer>
      explicit WriterBox(Writer writer);

      explicit WriterBox(WriterBox* writer);

      explicit WriterBox(const std::shared_ptr<WriterBox>& writer);

      explicit WriterBox(const std::unique_ptr<WriterBox>& writer);

      void Write(const void* data, std::size_t size);

      template<typename B>
      void Write(const B& data);

    private:
      struct VirtualWriter {
        virtual ~VirtualWriter() = default;
        virtual void Write(const void* data, std::size_t size) = 0;
        virtual void Write(const BufferView& data) = 0;
      };
      template<typename W>
      struct WrappedWriter final : VirtualWriter {
        using Writer = W;
        GetOptionalLocalPtr<Writer> m_writer;

        template<typename... Args>
        WrappedWriter(Args&&... args);
        void Write(const void* data, std::size_t size) override;
        void Write(const BufferView& data) override;
      };
      std::shared_ptr<VirtualWriter> m_writer;
  };

  template<typename T, typename... Args>
  WriterBox::WriterBox(std::in_place_type_t<T>, Args&&... args)
    : m_writer(std::make_shared<WrappedWriter<T>>(
        std::forward<Args>(args)...)) {}

  template<typename Writer>
  WriterBox::WriterBox(Writer writer)
    : WriterBox(std::in_place_type<Writer>, std::move<Writer>(writer)) {}

  inline WriterBox::WriterBox(WriterBox* writer)
    : m_writer(writer->m_writer) {}

  inline WriterBox::WriterBox(const std::shared_ptr<WriterBox>& writer)
    : WriterBox(writer.get()) {}

  inline WriterBox::WriterBox(const std::unique_ptr<WriterBox>& writer)
    : WriterBox(writer.get()) {}

  inline void WriterBox::Write(const void* data, std::size_t size) {
    m_writer->Write(data, size);
  }

  template<typename B>
  void WriterBox::Write(const B& data) {
    m_writer->Write(BufferView(data));
  }

  template<typename W>
  template<typename... Args>
  WriterBox::WrappedWriter<W>::WrappedWriter(Args&&... args)
    : m_writer(std::forward<Args>(args)...) {}

  template<typename W>
  void WriterBox::WrappedWriter<W>::Write(const void* data, std::size_t size) {
    m_writer->Write(data, size);
  }

  template<typename W>
  void WriterBox::WrappedWriter<W>::Write(const BufferView& data) {
    m_writer->Write(data);
  }
}

  template<typename Buffer>
  struct ImplementsConcept<IO::WriterBox, IO::Writer<Buffer>> :
    std::true_type {};
}

#endif
