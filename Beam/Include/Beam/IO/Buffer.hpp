#ifndef BEAM_BUFFER_BOX_HPP
#define BEAM_BUFFER_BOX_HPP
#include <array>
#include <concepts>
#include <ostream>
#include <string_view>
#include <type_traits>
#include <utility>
#include <boost/throw_exception.hpp>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Out.hpp"

namespace Beam {

  /** Concept satisfied by types that implement the const Buffer interface. */
  template<typename T>
  concept IsConstBuffer = requires(
    T & t, const T & ct, std::size_t index, std::size_t size,
    const void* source) {
      { ct.get_data() } -> std::convertible_to<const char*>;
      { ct.get_size() } -> std::convertible_to<std::size_t>;
  };

  /** Concept satisfied by types that implement the Buffer interface. */
  template<typename T>
  concept IsBuffer = IsConstBuffer<T> && requires(
      T& t, const T& ct, std::size_t index, std::size_t size,
      const void* source) {
    { t.get_mutable_data() } -> std::convertible_to<char*>;
    { t.grow(size) } -> std::convertible_to<std::size_t>;
    { t.shrink(size) } -> std::convertible_to<std::size_t>;
    { t.write(index, source, size) } -> std::same_as<void>;
  };

  /** Provides a generic interface to an arbitrary Buffer object. */
  class Buffer {
    public:

      /**
       * Constructs a Buffer in-place.
       * @tparam T The type of buffer to construct in place.
       * @param args The arguments to pass to the constructed buffer.
       */
      template<IsBuffer T, typename... Args>
      explicit Buffer(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a Buffer by copying an existing buffer.
       * @tparam T The type of buffer to copy.
       * @param buffer The buffer to copy.
       */
      template<IsBuffer T>
      Buffer(const T& buffer);

      Buffer(const Buffer&);

      /** Returns the raw data for read-only access. */
      const char* get_data() const;

      /** Returns the size, in bytes, of the data. */
      std::size_t get_size() const;

      /** Returns the raw data for read/write access. */
      char* get_mutable_data();

      /**
       * Grows this Buffer.
       * @param size The additional size to grow this Buffer by.
       * @return The actual size that the buffer grew by.
       */
      std::size_t grow(std::size_t size);

      /**
       * Shrinks this Buffer.
       * @param size The size to shrink the Buffer by.
       * @return The actual size that the buffer shrunk by.
       */
      std::size_t shrink(std::size_t size);

      /**
       * Writes to this Buffer.
       * @param index The index to begin writing to.
       * @param source The source to write from.
       * @param size The size to write.
       */
      void write(std::size_t index, const void* source, std::size_t size);

    private:
      struct VirtualBuffer {
        virtual ~VirtualBuffer() = default;

        virtual std::unique_ptr<VirtualBuffer> clone() = 0;
        virtual const char* get_data() const = 0;
        virtual std::size_t get_size() const = 0;
        virtual char* get_mutable_data() = 0;
        virtual std::size_t grow(std::size_t size) = 0;
        virtual std::size_t shrink(std::size_t size) = 0;
        virtual void write(
          std::size_t index, const void* source, std::size_t size) = 0;
      };
      template<typename B>
      struct WrappedBuffer final : VirtualBuffer {
        using Buffer = B;
        local_ptr_t<Buffer> m_buffer;

        template<typename... Args>
        WrappedBuffer(Args&&... args);

        std::unique_ptr<VirtualBuffer> clone() override;
        const char* get_data() const override;
        std::size_t get_size() const override;
        char* get_mutable_data() override;
        std::size_t grow(std::size_t size) override;
        std::size_t shrink(std::size_t size) override;
        void write(
          std::size_t index, const void* source, std::size_t size) override;
      };
      std::unique_ptr<VirtualBuffer> m_buffer;
  };

  /** Returns whether a Buffer is empty. */
  bool is_empty(const IsConstBuffer auto& buffer) {
    return buffer.get_size() == 0;
  }

  /** Returns the last number of bytes in a Buffer. */
  const char* get_suffix(const IsConstBuffer auto& buffer, std::size_t size) {
    if(size > buffer.get_size()) {
      boost::throw_with_location(
        std::out_of_range("Suffix size exceeds buffer size."));
    }
    return buffer.get_data() + (buffer.get_size() - size);
  }

  /** Returns the last number of bytes in a Buffer. */
  char* get_mutable_suffix(IsBuffer auto& buffer, std::size_t size) {
    if(size > buffer.get_size()) {
      boost::throw_with_location(
        std::out_of_range("Suffix size exceeds buffer size."));
    }
    return buffer.get_mutable_data() + (buffer.get_size() - size);
  }

  /** Resets a Buffer to an empty state. */
  void reset(IsBuffer auto& buffer) {
    buffer.shrink(buffer.get_size());
  }

  /**
   * Reserves space in a Buffer.
   * @param buffer The Buffer to reserve space in.
   * @param size The total amount of space that needs to be reserved.
   * @return The total size of the buffer after reserving space.
   */
  std::size_t reserve(IsBuffer auto& buffer, std::size_t size) {
    if(size > buffer.get_size()) {
      buffer.grow(size - buffer.get_size());
    }
    return buffer.get_size();
  }

  /**
   * Writes an object to a Buffer.
   * @param buffer The Buffer to write to.
   * @param index The index to begin writing to.
   * @param value The object to write.
   */
  template<typename T>
  void write(IsBuffer auto& buffer, std::size_t index, T value) {
    buffer.write(index, &value, sizeof(T));
  }

  /**
   * Writes data to the end of a Buffer.
   * @param buffer The Buffer to write to.
   * @param source The source to write from.
   * @param size The size to write.
   */
  void append(IsBuffer auto& buffer, const void* source, std::size_t size) {
    auto growth = buffer.grow(size);
    if(growth < size) {
      boost::throw_with_location(std::out_of_range("Buffer failed to grow."));
    }
    buffer.write(buffer.get_size() - size, source, size);
  }

  /**
   * Writes data to the end of a Buffer.
   * @param destination The Buffer to write to.
   * @param source The source to write from.
   */
  void append(IsBuffer auto& destination, const IsConstBuffer auto& source) {
    append(destination, source.get_data(), source.get_size());
  }

  /**
   * Writes an object to the end of a Buffer.
   * @param buffer The Buffer to write to.
   * @param value The object to write.
   */
  template<typename T> requires(!IsConstBuffer<T>)
  void append(IsBuffer auto& buffer, const T& value) {
    append(buffer, &value, sizeof(T));
  }

  /**
   * Writes data to the end of a Buffer up to a maximum number of bytes.
   * @param buffer The Buffer to write to.
   * @param source The source to write from.
   * @param size The size to write.
   * @return The actual number of bytes written.
   */
  std::size_t append_up_to(
      IsBuffer auto& buffer, const void* source, std::size_t size) {
    auto write_size = buffer.grow(size);
    buffer.write(buffer.get_size() - write_size, source, write_size);
    return write_size;
  }


  /**
   * Extracts a value from a Buffer.
   * @tparam B The Buffer type.
   * @tparam T The value type to extract.
   * @param buffer The buffer to read from.
   * @param index The index to begin reading from.
   * @param value Where the extracted value will be stored.
   */
  template<typename T>
  void extract(const IsBuffer auto& buffer, std::size_t index, Out<T> value) {
    std::memcpy(
      reinterpret_cast<char*>(&*value), buffer.get_data() + index, sizeof(T));
  }

  /**
   * Extracts a value from a Buffer.
   * @tparam B The Buffer type.
   * @tparam T The value type to extract.
   * @param buffer The buffer to read from.
   * @param index The index to begin reading from.
   */
  template<typename T>
  T extract(IsBuffer auto& buffer, std::size_t index) {
    auto value = T();
    extract(buffer, index, Store(value));
    return value;
  }

  /**
   * Constructs a Buffer from a string.
   * @param source The string.
   * @return A Buffer whose contents are a copy of the <i>source</i>.
   */
  template<IsBuffer B>
  B from(std::string_view source) {
    return B(source.data(), source.size());
  }

  /**
   * Tests two Buffers for equality.
   * @param lhs The left hand side of the equality.
   * @param rhs The right hand side of the equality.
   * @return <code>true</code> iff lhs and rhs have the same size and their
   *         bytes are identical.
   */
  bool operator ==(const IsBuffer auto& lhs, const IsBuffer auto& rhs) {
    if(lhs.get_size() != rhs.get_size()) {
      return false;
    }
    return std::memcmp(lhs.get_data(), rhs.get_data(), lhs.get_size()) == 0;
  }

  /**
   * Tests two Buffers for inequality.
   * @param lhs The left hand side of the inequality.
   * @param rhs The right hand side of the inequality.
   * @return <code>true</code> iff lhs and rhs have different sizes or their
   *         bytes are not identical.
   */
  bool operator !=(const IsBuffer auto& lhs, const IsBuffer auto& rhs) {
    return !(lhs == rhs);
  }

  /**
   * Tests a Buffer and a string for equality.
   * @param lhs The left hand side of the equality.
   * @param rhs The right hand side of the equality.
   * @return <code>true</code> iff lhs and rhs have the same size and their
   *         bytes are identical.
   */
  bool operator ==(const IsBuffer auto& lhs, std::string_view rhs) {
    if(lhs.get_size() != rhs.size()) {
      return false;
    }
    return std::memcmp(lhs.get_data(), rhs.data(), lhs.get_size()) == 0;
  }

  /**
   * Tests a Buffer and a string for inequality.
   * @param lhs The left hand side of the inequality.
   * @param rhs The right hand side of the inequality.
   * @return <code>true</code> iff lhs and rhs have different sizes or their
   *         bytes are not identical.
   */
  bool operator !=(const IsBuffer auto& lhs, std::string_view rhs) {
    return !(lhs == rhs);
  }

  /**
   * Tests a Buffer and a string for equality.
   * @param lhs The left hand side of the equality.
   * @param rhs The right hand side of the equality.
   * @return <code>true</code> iff lhs and rhs have the same size and their
   *         bytes are identical.
   */
  bool operator ==(std::string_view lhs, const IsBuffer auto& rhs) {
    return rhs == lhs;
  }

  /**
   * Tests a Buffer and a string for inequality.
   * @param lhs The left hand side of the inequality.
   * @param rhs The right hand side of the inequality.
   * @return <code>true</code> iff lhs and rhs have different sizes or their
   *         bytes are not identical.
   */
  bool operator !=(std::string_view lhs, const IsBuffer auto& rhs) {
    return !(lhs == rhs);
  }

  /**
   * Writes a Buffer to an ostream.
   * @param stream The stream to write to.
   * @param buffer The buffer to write.
   * @return A reference to <code>stream</code>.
   */
   std::ostream& operator <<(
      std::ostream& stream, const IsBuffer auto& buffer) {
    if(buffer.get_size() != 0) {
      stream.write(buffer.get_data(), buffer.get_size());
    }
    return stream;
  }

  /**
   * Encodes a Buffer to base64.
   * @param source The Buffer to encode.
   * @return The base64 encoding of the <i>source</i>.
   */
  std::string encode_base64(const IsBuffer auto& source) {
    static const auto CODES =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    auto result = std::string();
    for(auto i = std::size_t(0); i < source.get_size(); i += 3) {
      auto b = (source.get_data()[i] & 0xFC) >> 2;
      result += CODES[b];
      b = (source.get_data()[i] & 0x03) << 4;
      if(i + 1 < source.get_size()) {
        b |= (source.get_data()[i + 1] & 0xF0) >> 4;
        result += CODES[b];
        b = (source.get_data()[i + 1] & 0x0F) << 2;
        if(i + 2 < source.get_size()) {
          b |= (source.get_data()[i + 2] & 0xC0) >> 6;
          result += CODES[b];
          b = source.get_data()[i + 2] & 0x3F;
          result += CODES[b];
        } else  {
          result += CODES[b];
          result += '=';
        }
      } else {
        result += CODES[b];
        result += "==";
      }
    }
    return result;
  }

  /**
   * Decodes a base64 string into a Buffer.
   * @param source The string to decode.
   * @param buffer The Buffer to append the data to.
   */
  template<IsBuffer B>
  void decode_base64(std::string_view source, Out<B> buffer) {
    static const auto DECODING_TABLE = [] {
      auto table = std::array<std::uint8_t, 256>();
      table.fill(64);
      for(auto i = std::uint8_t(0); i < 64; ++i) {
        table["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
      }
      return table;
    }();
    auto source_size = source.size();
    while(source_size > 0 && source[source_size - 1] == '=') {
      --source_size;
    }
    auto output_length = (3 * source_size) / 4;
    reserve(*buffer, output_length);
    auto i = std::size_t(0);
    auto j = std::size_t(0);
    while(i < source_size) {
      auto a = i < source_size ?
        DECODING_TABLE[static_cast<unsigned char>(source[i])] : 64;
      ++i;
      auto b = i < source_size ?
        DECODING_TABLE[static_cast<unsigned char>(source[i])] : 64;
      ++i;
      auto c = i < source_size ?
        DECODING_TABLE[static_cast<unsigned char>(source[i])] : 64;
      ++i;
      auto d = i < source_size ?
        DECODING_TABLE[static_cast<unsigned char>(source[i])] : 64;
      ++i;
      auto triple = (a << 18) + (b << 12) + (c << 6) + d;
      if(j < output_length) {
        buffer->get_mutable_data()[j] = (triple >> 16) & 0xFF;
        ++j;
      }
      if(j < output_length) {
        buffer->get_mutable_data()[j] = (triple >> 8) & 0xFF;
        ++j;
      }
      if(j < output_length) {
        buffer->get_mutable_data()[j] = triple & 0xFF;
        ++j;
      }
    }
  }

  template<IsBuffer T, typename... Args>
  Buffer::Buffer(std::in_place_type_t<T>, Args&&... args)
    : m_buffer(
        std::make_unique<WrappedBuffer<T>>(std::forward<Args>(args)...)) {}

  template<IsBuffer T>
  Buffer::Buffer(const T& buffer)
    : m_buffer(std::make_unique<WrappedBuffer<T>>(buffer)) {}

  inline Buffer::Buffer(const Buffer& buffer)
    : m_buffer(buffer.m_buffer->clone()) {}

  inline const char* Buffer::get_data() const {
    return m_buffer->get_data();
  }

  inline std::size_t Buffer::get_size() const {
    return m_buffer->get_size();
  }

  inline char* Buffer::get_mutable_data() {
    return m_buffer->get_mutable_data();
  }

  inline std::size_t Buffer::grow(std::size_t size) {
    return m_buffer->grow(size);
  }

  inline std::size_t Buffer::shrink(std::size_t size) {
    return m_buffer->shrink(size);
  }

  inline void Buffer::write(
      std::size_t index, const void* source, std::size_t size) {
    m_buffer->write(index, source, size);
  }

  template<typename B>
  template<typename... Args>
  Buffer::WrappedBuffer<B>::WrappedBuffer(Args&&... args)
    : m_buffer(std::forward<Args>(args)...) {}

  template<typename B>
  std::unique_ptr<typename Buffer::VirtualBuffer>
      Buffer::WrappedBuffer<B>::clone() {
    return std::make_unique<WrappedBuffer>(*m_buffer);
  }

  template<typename B>
  const char* Buffer::WrappedBuffer<B>::get_data() const {
    return m_buffer->get_data();
  }

  template<typename B>
  std::size_t Buffer::WrappedBuffer<B>::get_size() const {
    return m_buffer->get_size();
  }

  template<typename B>
  char* Buffer::WrappedBuffer<B>::get_mutable_data() {
    return m_buffer->get_mutable_data();
  }

  template<typename B>
  std::size_t Buffer::WrappedBuffer<B>::grow(std::size_t size) {
    return m_buffer->grow(size);
  }

  template<typename B>
  std::size_t Buffer::WrappedBuffer<B>::shrink(std::size_t size) {
    return m_buffer->shrink(size);
  }

  template<typename B>
  void Buffer::WrappedBuffer<B>::write(
      std::size_t index, const void* source, std::size_t size) {
    m_buffer->write(index, source, size);
  }
}

#endif
