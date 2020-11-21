#ifndef BEAM_BUFFER_VIEW_HPP
#define BEAM_BUFFER_VIEW_HPP
#include <type_traits>
#include "Beam/IO/Buffer.hpp"

namespace Beam::IO {

  /** Provides a generic interface to an arbitrary const Buffer object. */
  class BufferView {
    public:
      template<typename Buffer, typename = std::enable_if_t<
        !std::is_base_of_v<BufferView, std::decay_t<Buffer>>>>
      explicit BufferView(const Buffer& buffer);

      bool IsEmpty() const;

      const char* GetData() const;

      std::size_t GetSize() const;

      template<typename T>
      void Extract(std::size_t index, Out<T> value) const;

      template<typename T>
      T Extract(std::size_t index) const;

    private:
      struct VirtualBuffer {
        virtual ~VirtualBuffer() = default;
        virtual bool IsEmpty() const = 0;
        virtual const char* GetData() const = 0;
        virtual std::size_t GetSize() const = 0;
      };
      template<typename B>
      struct WrappedBuffer final : VirtualBuffer {
        using Buffer = B;
        const Buffer* m_buffer;

        WrappedBuffer(const Buffer& buffer);
        bool IsEmpty() const override;
        const char* GetData() const override;
        std::size_t GetSize() const override;
      };
      std::shared_ptr<VirtualBuffer> m_buffer;
  };

  template<typename Buffer, typename>
  BufferView::BufferView(const Buffer& buffer)
    : m_buffer(std::make_unique<WrappedBuffer<std::decay_t<Buffer>>>(buffer)) {}

  inline bool BufferView::IsEmpty() const {
    return m_buffer->IsEmpty();
  }

  inline const char* BufferView::GetData() const {
    return m_buffer->GetData();
  }

  inline std::size_t BufferView::GetSize() const {
    return m_buffer->GetSize();
  }

  template<typename T>
  void BufferView::Extract(std::size_t index, Out<T> value) const {
    std::memcpy(reinterpret_cast<char*>(&*value), GetData() + index, sizeof(T));
  }

  template<typename T>
  T BufferView::Extract(std::size_t index) const {
    auto value = T();
    Extract(index, Store(value));
    return value;
  }

  template<typename B>
  BufferView::WrappedBuffer<B>::WrappedBuffer(const Buffer& buffer)
    : m_buffer(&buffer) {}

  template<typename B>
  bool BufferView::WrappedBuffer<B>::IsEmpty() const {
    return m_buffer->IsEmpty();
  }

  template<typename B>
  const char* BufferView::WrappedBuffer<B>::GetData() const {
    return m_buffer->GetData();
  }

  template<typename B>
  std::size_t BufferView::WrappedBuffer<B>::GetSize() const {
    return m_buffer->GetSize();
  }
}

#endif
