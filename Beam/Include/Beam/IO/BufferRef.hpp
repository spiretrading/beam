#ifndef BEAM_BUFFER_REF_HPP
#define BEAM_BUFFER_REF_HPP
#include "Beam/IO/Buffer.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** Implements a non-owning, type-erased reference to a Buffer. */
  class BufferRef {
    public:

      /**
       * Constructs a BufferRef to provided buffer.
       * @param buffer The buffer to reference.
       */
      template<DisableCopy<BufferRef> T> requires IsBuffer<T>
      BufferRef(T& buffer) noexcept;

      BufferRef(const BufferRef&) = default;

      const char* get_data() const;
      std::size_t get_size() const;
      char* get_mutable_data();
      std::size_t grow(std::size_t size);
      std::size_t shrink(std::size_t size);
      void write(std::size_t index, const void* source, std::size_t size);

    private:
      struct Table {
        const char* (*m_get_data)(const void*);
        std::size_t (*m_get_size)(const void*);
        char* (*m_get_mutable_data)(void*);
        std::size_t (*m_grow)(void*, std::size_t);
        std::size_t (*m_shrink)(void*, std::size_t);
        void (*m_write)(void*, std::size_t, const void*, std::size_t);
      };
      void* m_buffer;
      const Table* m_table;

      template<IsBuffer B>
      static const Table& get_table() noexcept;
  };

  /** Implements a non-owning, type-erased reference to a ConstBuffer. */
  class BufferCRef {
  public:

    /**
     * Constructs a BufferCRef to provided buffer.
     * @param buffer The buffer to reference.
     */
    template<IsConstBuffer T>
    BufferCRef(const T& buffer) noexcept;

    BufferCRef(const BufferCRef& buffer) = default;

    const char* get_data() const;
    std::size_t get_size() const;

  private:
    struct Table {
      const char* (*m_get_data)(const void*);
      std::size_t(*m_get_size)(const void*);
    };

    template<IsConstBuffer B>
    static const Table& get_table() noexcept;
    const void* m_buffer;
    const Table* m_table;
  };

  template<DisableCopy<BufferRef> T> requires IsBuffer<T>
  BufferRef::BufferRef(T& buffer) noexcept
    : m_buffer(&buffer),
      m_table(&get_table<T>()) {}

  inline const char* BufferRef::get_data() const {
    return m_table->m_get_data(m_buffer);
  }

  inline std::size_t BufferRef::get_size() const {
    return m_table->m_get_size(m_buffer);
  }

  inline char* BufferRef::get_mutable_data() {
    return m_table->m_get_mutable_data(m_buffer);
  }

  inline std::size_t BufferRef::grow(std::size_t size) {
    return m_table->m_grow(m_buffer, size);
  }

  inline std::size_t BufferRef::shrink(std::size_t size) {
    return m_table->m_shrink(m_buffer, size);
  }

  inline void BufferRef::write(
      std::size_t index, const void* source, std::size_t size) {
    m_table->m_write(m_buffer, index, source, size);
  }

  template<IsBuffer B>
  const BufferRef::Table& BufferRef::get_table() noexcept {
    static const auto TABLE = Table(
      +[] (const void* buffer) -> const char* {
        return static_cast<const B*>(buffer)->get_data();
      },
      +[] (const void* buffer) -> std::size_t {
        return static_cast<const B*>(buffer)->get_size();
      },
      +[] (void* buffer) -> char* {
        return static_cast<B*>(buffer)->get_mutable_data();
      },
      +[] (void* buffer, std::size_t size) {
        return static_cast<B*>(buffer)->grow(size);
      },
      +[] (void* buffer, std::size_t size) {
        return static_cast<B*>(buffer)->shrink(size);
      },
      +[] (void* buffer, std::size_t index, const void* source,
          std::size_t size) {
        static_cast<B*>(buffer)->write(index, source, size);
      });
    return TABLE;
  }

  template<IsConstBuffer T>
  BufferCRef::BufferCRef(const T& buffer) noexcept
    : m_buffer(&buffer),
      m_table(&get_table<T>()) {}

  inline const char* BufferCRef::get_data() const {
    return m_table->m_get_data(m_buffer);
  }

  inline std::size_t BufferCRef::get_size() const {
    return m_table->m_get_size(m_buffer);
  }

  template<IsConstBuffer B>
  const BufferCRef::Table& BufferCRef::get_table() noexcept {
    static const auto TABLE = Table(
      +[] (const void* buffer) -> const char* {
        return static_cast<const B*>(buffer)->get_data();
      },
      +[] (const void* buffer) -> std::size_t {
        return static_cast<const B*>(buffer)->get_size();
      });
    return TABLE;
  }
}

#endif
