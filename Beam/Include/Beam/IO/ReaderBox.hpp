#ifndef BEAM_READER_BOX_HPP
#define BEAM_READER_BOX_HPP
#include <memory>
#include <type_traits>
#include "Beam/IO/BufferBox.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /** Provides a generic interface over an arbitrary Reader. */
  class ReaderBox {
    public:

      /**
       * Constructs a ReaderBox of a specified type using emplacement.
       * @param <T> The type of reader to emplace.
       * @param args The arguments to pass to the emplaced reader.
       */
      template<typename T, typename... Args>
      explicit ReaderBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ReaderBox by copying an existing reader.
       * @param reader The reader to copy.
       */
      template<typename Reader>
      explicit ReaderBox(Reader reader);

      explicit ReaderBox(ReaderBox* reader);

      explicit ReaderBox(const std::shared_ptr<ReaderBox>& reader);

      explicit ReaderBox(const std::unique_ptr<ReaderBox>& reader);

      bool IsDataAvailable() const;

      template<typename Buffer>
      std::size_t Read(Out<Buffer> destination);

      std::size_t Read(char* destination, std::size_t size);

      template<typename Buffer>
      std::size_t Read(Out<Buffer> destination, std::size_t size);

    private:
      struct VirtualReader {
        virtual ~VirtualReader() = default;
        virtual bool IsDataAvailable() const = 0;
        virtual std::size_t Read(Out<BufferBox> destination) = 0;
        virtual std::size_t Read(char* destination, std::size_t size) = 0;
        virtual std::size_t Read(Out<BufferBox> destination,
          std::size_t size) = 0;
      };
      template<typename R>
      struct WrappedReader final : VirtualReader {
        using Reader = R;
        GetOptionalLocalPtr<Reader> m_reader;

        template<typename... Args>
        WrappedReader(Args&&... args);
        bool IsDataAvailable() const override;
        std::size_t Read(Out<BufferBox> destination) override;
        std::size_t Read(char* destination, std::size_t size) override;
        std::size_t Read(Out<BufferBox> destination, std::size_t size) override;
      };
      std::shared_ptr<VirtualReader> m_reader;
  };

  template<typename T, typename... Args>
  ReaderBox::ReaderBox(std::in_place_type_t<T>, Args&&... args)
    : m_reader(std::make_shared<WrappedReader<T>>(
        std::forward<Args>(args)...)) {}

  template<typename Reader>
  ReaderBox::ReaderBox(Reader reader)
    : ReaderBox(std::in_place_type<Reader>, std::move(reader)) {}

  inline ReaderBox::ReaderBox(ReaderBox* reader)
    : ReaderBox(*reader) {}

  inline ReaderBox::ReaderBox(const std::shared_ptr<ReaderBox>& reader)
    : ReaderBox(*reader) {}

  inline ReaderBox::ReaderBox(const std::unique_ptr<ReaderBox>& reader)
    : ReaderBox(*reader) {}

  inline bool ReaderBox::IsDataAvailable() const {
    return m_reader->IsDataAvailable();
  }

  template<typename Buffer>
  std::size_t ReaderBox::Read(Out<Buffer> destination) {
    auto box = BufferBox(&*destination);
    return m_reader->Read(Store(box));
  }

  inline std::size_t ReaderBox::Read(char* destination, std::size_t size) {
    return m_reader->Read(destination, size);
  }

  template<typename Buffer>
  std::size_t ReaderBox::Read(Out<Buffer> destination, std::size_t size) {
    auto box = BufferBox(&*destination);
    return m_reader->Read(Store(box), size);
  }

  template<typename R>
  template<typename... Args>
  ReaderBox::WrappedReader<R>::WrappedReader(Args&&... args)
    : m_reader(std::forward<Args>(args)...) {}

  template<typename R>
  bool ReaderBox::WrappedReader<R>::IsDataAvailable() const {
    return m_reader->IsDataAvailable();
  }

  template<typename R>
  std::size_t ReaderBox::WrappedReader<R>::Read(Out<BufferBox> destination) {
    return m_reader->Read(Store(destination));
  }

  template<typename R>
  std::size_t ReaderBox::WrappedReader<R>::Read(char* destination,
      std::size_t size) {
    return m_reader->Read(destination, size);
  }

  template<typename R>
  std::size_t ReaderBox::WrappedReader<R>::Read(Out<BufferBox> destination,
      std::size_t size) {
    return m_reader->Read(Store(destination), size);
  }
}

  template<>
  struct ImplementsConcept<IO::ReaderBox, IO::Reader> : std::true_type {};
}

#endif
