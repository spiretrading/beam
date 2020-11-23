#ifndef BEAM_BUFFER_BOX_HPP
#define BEAM_BUFFER_BOX_HPP
#include <type_traits>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /** Provides a generic interface to an arbitrary Buffer object. */
  class BufferBox {
    public:
      template<typename Buffer, typename = std::enable_if_t<
        !std::is_base_of_v<BufferBox, std::decay_t<Buffer>>>>
      explicit BufferBox(Buffer&& buffer);

      explicit BufferBox(BufferBox* buffer);

      explicit BufferBox(const std::shared_ptr<BufferBox>& buffer);

      explicit BufferBox(const std::unique_ptr<BufferBox>& buffer);

      bool IsEmpty() const;

      void Grow(std::size_t size);

      void Shrink(std::size_t size);

      void ShrinkFront(std::size_t size);

      void Reserve(std::size_t size);

      void Write(std::size_t index, const void* source, std::size_t size);

      template<typename T>
      void Write(std::size_t index, T value);

      template<typename Buffer>
      std::enable_if_t<ImplementsConcept<Buffer, IO::Buffer>::value> Append(
        const Buffer& buffer);

      void Append(const void* data, std::size_t size);

      template<typename T>
      std::enable_if_t<!ImplementsConcept<T, IO::Buffer>::value> Append(
        T value);

      void Reset();

      const char* GetData() const;

      char* GetMutableData();

      std::size_t GetSize() const;

      template<typename T>
      void Extract(std::size_t index, Out<T> value) const;

      template<typename T>
      T Extract(std::size_t index) const;

    private:
      struct VirtualBuffer {
        virtual ~VirtualBuffer() = default;
        virtual bool IsEmpty() const = 0;
        virtual void Grow(std::size_t size) = 0;
        virtual void Shrink(std::size_t size) = 0;
        virtual void ShrinkFront(std::size_t size) = 0;
        virtual void Reserve(std::size_t size) = 0;
        virtual void Write(std::size_t index, const void* source,
          std::size_t size) = 0;
        virtual void Append(const void* data, std::size_t size) = 0;
        virtual void Reset() = 0;
        virtual const char* GetData() const = 0;
        virtual char* GetMutableData() = 0;
        virtual std::size_t GetSize() const = 0;
      };
      template<typename B>
      struct WrappedBuffer final : VirtualBuffer {
        using Buffer = B;
        GetOptionalLocalPtr<Buffer> m_buffer;

        template<typename... Args>
        WrappedBuffer(Args&&... args);
        bool IsEmpty() const override;
        void Grow(std::size_t size) override;
        void Shrink(std::size_t size) override;
        void ShrinkFront(std::size_t size) override;
        void Reserve(std::size_t size) override;
        void Write(std::size_t index, const void* source,
          std::size_t size) override;
        void Append(const void* data, std::size_t size) override;
        void Reset() override;
        const char* GetData() const override;
        char* GetMutableData() override;
        std::size_t GetSize() const override;
      };
      std::shared_ptr<VirtualBuffer> m_buffer;
  };

  template<typename Buffer, typename>
  BufferBox::BufferBox(Buffer&& buffer)
    : m_buffer(std::make_unique<WrappedBuffer<std::decay_t<Buffer>>>(
        std::forward<Buffer>(buffer))) {}

  inline BufferBox::BufferBox(BufferBox* buffer)
    : BufferBox(*buffer) {}

  inline BufferBox::BufferBox(const std::shared_ptr<BufferBox>& buffer)
    : BufferBox(*buffer) {}

  inline BufferBox::BufferBox(const std::unique_ptr<BufferBox>& buffer)
    : BufferBox(*buffer) {}

  inline bool BufferBox::IsEmpty() const {
    return m_buffer->IsEmpty();
  }

  inline void BufferBox::Grow(std::size_t size) {
    m_buffer->Grow(size);
  }

  inline void BufferBox::Shrink(std::size_t size) {
    m_buffer->Shrink(size);
  }

  inline void BufferBox::ShrinkFront(std::size_t size) {
    m_buffer->ShrinkFront(size);
  }

  inline void BufferBox::Reserve(std::size_t size) {
    m_buffer->Reserve(size);
  }

  inline void BufferBox::Write(std::size_t index, const void* source,
      std::size_t size) {
    m_buffer->Write(index, source, size);
  }

  template<typename T>
  void BufferBox::Write(std::size_t index, T value) {
    Write(index, &value, sizeof(T));
  }

  template<typename Buffer>
  std::enable_if_t<ImplementsConcept<Buffer, IO::Buffer>::value>
      BufferBox::Append(const Buffer& buffer) {
    Append(buffer.GetData(), buffer.GetSize());
  }

  inline void BufferBox::Append(const void* data, std::size_t size) {
    m_buffer->Append(data, size);
  }

  template<typename T>
  std::enable_if_t<!ImplementsConcept<T, IO::Buffer>::value> BufferBox::Append(
      T value) {
    Append(&value, sizeof(T));
  }

  inline void BufferBox::Reset() {
    m_buffer->Reset();
  }

  inline const char* BufferBox::GetData() const {
    return m_buffer->GetData();
  }

  inline char* BufferBox::GetMutableData() {
    return m_buffer->GetMutableData();
  }

  inline std::size_t BufferBox::GetSize() const {
    return m_buffer->GetSize();
  }

  template<typename T>
  void BufferBox::Extract(std::size_t index, Out<T> value) const {
    std::memcpy(reinterpret_cast<char*>(&*value), GetData() + index, sizeof(T));
  }

  template<typename T>
  T BufferBox::Extract(std::size_t index) const {
    auto value = T();
    Extract(index, Store(value));
    return value;
  }

  template<typename B>
  template<typename... Args>
  BufferBox::WrappedBuffer<B>::WrappedBuffer(Args&&... args)
    : m_buffer(std::forward<Args>(args)...) {}

  template<typename B>
  bool BufferBox::WrappedBuffer<B>::IsEmpty() const {
    return m_buffer->IsEmpty();
  }

  template<typename B>
  void BufferBox::WrappedBuffer<B>::Grow(std::size_t size) {
    m_buffer->Grow(size);
  }

  template<typename B>
  void BufferBox::WrappedBuffer<B>::Shrink(std::size_t size) {
    m_buffer->Shrink(size);
  }

  template<typename B>
  void BufferBox::WrappedBuffer<B>::ShrinkFront(std::size_t size) {
    m_buffer->ShrinkFront(size);
  }

  template<typename B>
  void BufferBox::WrappedBuffer<B>::Reserve(std::size_t size) {
    m_buffer->Reserve(size);
  }

  template<typename B>
  void BufferBox::WrappedBuffer<B>::Write(std::size_t index, const void* source,
      std::size_t size) {
    m_buffer->Write(index, source, size);
  }

  template<typename B>
  void BufferBox::WrappedBuffer<B>::Append(const void* data, std::size_t size) {
    m_buffer->Append(data, size);
  }

  template<typename B>
  void BufferBox::WrappedBuffer<B>::Reset() {
    m_buffer->Reset();
  }

  template<typename B>
  const char* BufferBox::WrappedBuffer<B>::GetData() const {
    return m_buffer->GetData();
  }

  template<typename B>
  char* BufferBox::WrappedBuffer<B>::GetMutableData() {
    return m_buffer->GetMutableData();
  }

  template<typename B>
  std::size_t BufferBox::WrappedBuffer<B>::GetSize() const {
    return m_buffer->GetSize();
  }
}

  template<>
  struct ImplementsConcept<IO::BufferBox, IO::Buffer> : std::true_type {};
}

#endif
