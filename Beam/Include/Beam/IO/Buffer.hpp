#ifndef BEAM_BUFFER_HPP
#define BEAM_BUFFER_HPP
#include <cstring>
#include <ostream>
#include "Beam/IO/IO.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Utilities/Concept.hpp"

namespace Beam::IO {

  /** Defines a type that stores/manages a raw buffer of data. */
  struct Buffer : Concept<Buffer> {

    /** Returns <code>true</code> iff GetSize() == 0. */
    bool IsEmpty() const;

    /**
     * Grows this Buffer.
     * @param size The additional size to grow this Buffer by.
     */
    void Grow(std::size_t size);

    /**
     * Shrinks this Buffer.
     * @param size The size to shrink the Buffer by.
     */
    void Shrink(std::size_t size);

    /**
     * Shrinks this Buffer from the front.
     * @param size The size to shrink the Buffer by.
     */
    void ShrinkFront(std::size_t size);

    /**
     * Ensures this Buffer has a certain amount of space.
     * @param size The total amount of space that needs to be reserved.
     */
    void Reserve(std::size_t size);

    /**
     * Writes to this Buffer.
     * @param index The index to begin writing to.
     * @param source The source to write from.
     * @param size The size to write.
     */
    void Write(std::size_t index, const void* source, std::size_t size);

    /**
     * Writes a raw binary value to this Buffer.
     * @param <T> The type of value to write.
     * @param index The index to begin writing to.
     * @param value The value to write.
     */
    template<typename T>
    void Write(std::size_t index, T value);

    /**
     * Appends a Buffer.
     * @param buffer The Buffer to append.
     */
    void Append(const Buffer& buffer);

    /**
     * Appends a Buffer.
     * @param <B> Implements the Buffer Concept.
     * @param buffer The Buffer to append.
     */
    template<typename B>
    void Append(const B& buffer);

    /**
     * Appends raw data.
     * @param data The data to append.
     * @param size The size of the data to append.
     */
    void Append(const void* data, std::size_t size);

    /**
     * Appends a raw binary value to this Buffer.
     * @param <T> The type of value to append.
     * @param value The value to append.
     */
    template<typename T>
    void Append(T value);

    /** Resets this Buffer. */
    void Reset();

    /** Returns the raw data for read-only access. */
    const char* GetData() const;

    /** Returns the raw data for read/write access. */
    char* GetMutableData();

    /** Returns the size, in bytes, of the data. */
    std::size_t GetSize() const;

    /**
     * Extracts a value from the buffer.
     * @param index The index where the value is located.
     * @param value Where to store the value.
     */
    template<typename T>
    void Extract(std::size_t index, Out<T> value) const;

    /**
     * Extracts a value from the buffer.
     * @param index The index where the value is located.
     * @return The extracted value.
     */
    template<typename T>
    T Extract(std::size_t index) const;

    /**
     * Assigns a Buffer to this.
     * @param rhs The Buffer to assign to this.
     */
    Buffer& operator =(const Buffer& rhs);

    /**
     * Assigns a Buffer to this.
     * @param <B> Implements the Buffer Concept.
     * @param rhs The buffer to assign to this.
     */
    template<typename B>
    Buffer& operator =(const B& rhs);

    /**
     * Acquires a Buffer.
     * @param rhs The Buffer to acquire.
     */
    Buffer& operator =(Buffer&& rhs);
  };

  /**
   * Constructs a Buffer from a string.
   * @param source The string.
   * @return A Buffer whose contents are a copy of the <i>source</i>.
   */
  template<typename B>
  std::enable_if_t<ImplementsConcept<B, Buffer>::value, B> BufferFromString(
      const std::string& source) {
    return B(source.c_str(), source.size());
  }

  /**
   * Tests two Buffers for equality.
   * @param lhs The left hand side of the equality.
   * @param rhs The right hand side of the equality.
   * @return <code>true</code> iff lhs and rhs have the same size and their
   *         bytes are identical.
   */
  template<typename L, typename R>
  std::enable_if_t<ImplementsConcept<L, Buffer>::value &&
      ImplementsConcept<R, Buffer>::value, bool> operator ==(
      const L& lhs, const R& rhs) {
    if(lhs.GetSize() != rhs.GetSize()) {
      return false;
    }
    return std::memcmp(lhs.GetData(), rhs.GetData(), lhs.GetSize()) == 0;
  }

  /**
   * Tests two Buffers for inequality.
   * @param lhs The left hand side of the inequality.
   * @param rhs The right hand side of the inequality.
   * @return <code>true</code> iff lhs and rhs have different sizes or their
   *         bytes are not identical.
   */
  template<typename L, typename R>
  std::enable_if_t<ImplementsConcept<L, Buffer>::value &&
      ImplementsConcept<R, Buffer>::value, bool> operator !=(
      const L& lhs, const R& rhs) {
    return !(lhs == rhs);
  }

  /**
   * Tests a Buffer and a string for equality.
   * @param lhs The left hand side of the equality.
   * @param rhs The right hand side of the equality.
   * @return <code>true</code> iff lhs and rhs have the same size and their
   *         bytes are identical.
   */
  template<typename B>
  std::enable_if_t<ImplementsConcept<B, Buffer>::value, bool>
      operator ==(const B& lhs, const std::string& rhs) {
    if(lhs.GetSize() != rhs.size()) {
      return false;
    }
    return std::memcmp(lhs.GetData(), rhs.c_str(), lhs.GetSize()) == 0;
  }

  /**
   * Tests a Buffer and a string for inequality.
   * @param lhs The left hand side of the inequality.
   * @param rhs The right hand side of the inequality.
   * @return <code>true</code> iff lhs and rhs have different sizes or their
   *         bytes are not identical.
   */
  template<typename B>
  std::enable_if_t<ImplementsConcept<B, Buffer>::value, bool>
      operator !=(const B& lhs, const std::string& rhs) {
    return !(lhs == rhs);
  }

  /**
   * Tests a Buffer and a string for equality.
   * @param lhs The left hand side of the equality.
   * @param rhs The right hand side of the equality.
   * @return <code>true</code> iff lhs and rhs have the same size and their
   *         bytes are identical.
   */
  template<typename B>
  std::enable_if_t<ImplementsConcept<B, Buffer>::value, bool>
      operator ==(const std::string& lhs, const B& rhs) {
    return rhs == lhs;
  }

  /**
   * Tests a Buffer and a string for inequality.
   * @param lhs The left hand side of the inequality.
   * @param rhs The right hand side of the inequality.
   * @return <code>true</code> iff lhs and rhs have different sizes or their
   *         bytes are not identical.
   */
  template<typename B>
  std::enable_if_t<ImplementsConcept<B, Buffer>::value, bool>
      operator !=(const std::string& lhs, const B& rhs) {
    return rhs != lhs;
  }

  /**
   * Writes a Buffer to an ostream.
   * @param stream The stream to write to.
   * @param buffer The buffer to write.
   * @return A reference to <code>stream</code>.
   */
  template<typename B>
  std::enable_if_t<ImplementsConcept<B, Buffer>::value, std::ostream>&
      operator <<(std::ostream& stream, const B& buffer) {
    stream.write(buffer.GetData(), buffer.GetSize());
    return stream;
  }

  /**
   * Encodes a Buffer to base64.
   * @param source The Buffer to encode.
   * @return The base64 encoding of the <i>source</i>.
   */
  template<typename B>
  std::enable_if_t<ImplementsConcept<B, Buffer>::value, std::string>
      Base64Encode(const B& source) {
    static const auto CODES =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    std::string result;
    for(std::size_t i = 0; i < source.GetSize(); i += 3) {
      auto b = (source.GetData()[i] & 0xFC) >> 2;
      result += CODES[b];
      b = (source.GetData()[i] & 0x03) << 4;
      if(i + 1 < source.GetSize()) {
        b |= (source.GetData()[i + 1] & 0xF0) >> 4;
        result += CODES[b];
        b = (source.GetData()[i + 1] & 0x0F) << 2;
        if(i + 2 < source.GetSize()) {
          b |= (source.GetData()[i + 2] & 0xC0) >> 6;
          result += CODES[b];
          b = source.GetData()[i + 2] & 0x3F;
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
  template<typename B>
  std::enable_if_t<ImplementsConcept<B, Buffer>::value> Base64Decode(
    const std::string& source, Out<B> buffer) {}
}

#endif
