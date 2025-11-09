#ifndef BEAM_WRITER_HPP
#define BEAM_WRITER_HPP
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include "Beam/IO/BufferRef.hpp"
#include "Beam/IO/ValueSpan.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** Concept satisfied by types that implement the Writer interface. */
  template<typename T>
  concept IsWriter = requires(T& t, BufferCRef buffer) {
    { t.write(buffer) } -> std::same_as<void>;
  };

  /** Interface for writing data to a resource. */
  class Writer {
    public:

      /**
       * Constructs a Writer of a specified type using emplacement.
       * @tparam T The type of writer to emplace.
       * @param args The arguments to pass to the emplaced writer.
       */
      template<IsWriter T, typename... Args>
      explicit Writer(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a Writer by referencing an existing writer.
       * @param writer The writer to reference.
       */
      template<DisableCopy<Writer> T> requires IsWriter<dereference_t<T>>
      Writer(T&& writer);

      Writer(const Writer&) = default;
      Writer(Writer&&) = default;

      /**
       * Writes data to the resource.
       * @param data The data to write.
       */
      template<IsConstBuffer B>
      void write(const B& data);

    private:
      struct VirtualWriter {
        virtual ~VirtualWriter() = default;

        virtual void write(BufferCRef) = 0;
      };
      template<typename W>
      struct WrappedWriter final : VirtualWriter {
        using Writer = W;
        local_ptr_t<Writer> m_writer;

        template<typename... Args>
        WrappedWriter(Args&&... args);

        void write(BufferCRef data) override;
      };
      VirtualPtr<VirtualWriter> m_writer;
  };

  /**
   * Writes an object to a Writer.
   * @param writer The Writer to write to.
   * @param value The object to write.
   */
  template<IsWriter W, typename T> requires
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
  void write(W& writer, const T& value) {
    writer.write(ValueSpan(Ref(value)));
  }

  template<IsWriter T, typename... Args>
  Writer::Writer(std::in_place_type_t<T>, Args&&... args)
    : m_writer(
        make_virtual_ptr<WrappedWriter<T>>(std::forward<Args>(args)...)) {}

  template<DisableCopy<Writer> T> requires IsWriter<dereference_t<T>>
  Writer::Writer(T&& writer)
    : m_writer(make_virtual_ptr<WrappedWriter<std::remove_cvref_t<T>>>(
        std::forward<T>(writer))) {}

  template<IsConstBuffer B>
  void Writer::write(const B& data) {
    m_writer->write(data);
  }

  template<typename W>
  template<typename... Args>
  Writer::WrappedWriter<W>::WrappedWriter(Args&&... args)
    : m_writer(std::forward<Args>(args)...) {}

  template<typename W>
  void Writer::WrappedWriter<W>::write(BufferCRef data) {
    m_writer->write(data);
  }
}

#endif
