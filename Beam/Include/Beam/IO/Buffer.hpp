#ifndef BEAM_BUFFER_HPP
#define BEAM_BUFFER_HPP
#include <cstring>
#include <ostream>
#include "Beam/IO/IO.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Utilities/Concept.hpp"

namespace Beam {
namespace IO {

  /*! \struct Buffer
      \brief Defines a type that stores/manages a raw buffer of data.
   */
  struct Buffer : Concept<Buffer> {

    //! Assigns a Buffer to this.
    /*!
      \param rhs The Buffer to assign to this.
    */
    Buffer& operator =(const Buffer& rhs);

    //! Assigns a Buffer to this.
    /*!
      \tparam BufferType Implements the Buffer Concept.
      \param rhs The buffer to assign to this.
    */
    template<typename BufferType>
    Buffer& operator =(const BufferType& rhs);

    //! Acquires a Buffer.
    /*!
      \param rhs The Buffer to acquire.
    */
    Buffer& operator =(Buffer&& rhs);

    //! Returns <code>true</code> iff GetSize() == 0.
    bool IsEmpty() const;

    //! Grows this Buffer.
    /*!
      \param size The additional size to grow this Buffer by.
    */
    void Grow(std::size_t size);

    //! Shrinks this Buffer.
    /*!
      \param size The size to shrink the Buffer by.
    */
    void Shrink(std::size_t size);

    //! Shrinks this Buffer from the front.
    /*!
      \param size The size to shrink the Buffer by.
    */
    void ShrinkFront(std::size_t size);

    //! Ensures this Buffer has a certain amount of space.
    /*!
      \param size The total amount of space that needs to be reserved.
    */
    void Reserve(std::size_t size);

    //! Writes to this Buffer.
    /*!
      \param index The index to begin writing to.
      \param source The source to write from.
      \param size The size to write.
    */
    void Write(std::size_t index, const void* source, std::size_t size);

    //! Writes a raw binary value to this Buffer.
    /*!
      \tparam The type of value to write.
      \param index The index to begin writing to.
      \param value The value to write.
    */
    template<typename T>
    void Write(std::size_t index, T value);

    //! Appends a Buffer.
    /*!
      \param buffer The Buffer to append.
    */
    void Append(const Buffer& buffer);

    //! Appends a Buffer.
    /*!
      \tparam BufferType Implements the Buffer Concept.
      \param buffer The Buffer to append.
    */
    template<typename BufferType>
    void Append(const BufferType& buffer);

    //! Appends raw data.
    /*!
      \param data The data to append.
      \param size The size of the data to append.
    */
    void Append(const void* data, std::size_t size);

    //! Appends a raw binary value to this Buffer.
    /*!
      \tparam The type of value to append.
      \param value The value to append.
    */
    template<typename T>
    void Append(T value);

    //! Resets this Buffer.
    void Reset();

    //! Returns the raw data for read-only access.
    const char* GetData() const;

    //! Returns the raw data for read/write access.
    char* GetMutableData();

    //! Returns the size, in bytes, of the data.
    std::size_t GetSize() const;

    //! Extracts a value from the buffer.
    /*!
      \param index The index where the value is located.
      \param value Where to store the value.
    */
    template<typename T>
    void Extract(std::size_t index, Out<T> value) const;

    //! Extracts a value from the buffer.
    /*!
      \param index The index where the value is located.
      \return The extracted value.
    */
    template<typename T>
    T Extract(std::size_t index) const;

    //! Exchanges the contents of this Buffer.
    /*!
      \param buffer The Buffer to swap with.
    */
    void Swap(Buffer& buffer);
  };

  //! Constructs a Buffer from a string.
  /*!
    \param source The string.
    \return A Buffer whose contents are a copy of the <i>source</i>.
  */
  template<typename BufferType>
  typename std::enable_if<ImplementsConcept<BufferType, Buffer>::value,
      BufferType>::type BufferFromString(const std::string& source) {
    return BufferType(source.c_str(), source.size());
  }

  //! Tests two Buffers for equality.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff lhs and rhs have the same size and their
            bytes are identical.
  */
  template<typename LeftBuffer, typename RightBuffer>
  typename std::enable_if<ImplementsConcept<LeftBuffer, Buffer>::value &&
      ImplementsConcept<RightBuffer, Buffer>::value, bool>::type operator ==(
      const LeftBuffer& lhs, const RightBuffer& rhs) {
    if(lhs.GetSize() != rhs.GetSize()) {
      return false;
    }
    return std::memcmp(lhs.GetData(), rhs.GetData(), lhs.GetSize()) == 0;
  }

  //! Tests two Buffers for inequality.
  /*!
    \param lhs The left hand side of the inequality.
    \param rhs The right hand side of the inequality.
    \return <code>true</code> iff lhs and rhs have different sizes or their
            bytes are not identical.
  */
  template<typename LeftBuffer, typename RightBuffer>
  typename std::enable_if<ImplementsConcept<LeftBuffer, Buffer>::value &&
      ImplementsConcept<RightBuffer, Buffer>::value, bool>::type operator !=(
      const LeftBuffer& lhs, const RightBuffer& rhs) {
    return !(lhs == rhs);
  }

  //! Tests a Buffer and a string for equality.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff lhs and rhs have the same size and their
            bytes are identical.
  */
  template<typename BufferType>
  typename std::enable_if<ImplementsConcept<BufferType, Buffer>::value,
      bool>::type operator ==(const BufferType& lhs, const std::string& rhs) {
    if(lhs.GetSize() != rhs.size()) {
      return false;
    }
    return std::memcmp(lhs.GetData(), rhs.c_str(), lhs.GetSize()) == 0;
  }

  //! Tests a Buffer and a string for inequality.
  /*!
    \param lhs The left hand side of the inequality.
    \param rhs The right hand side of the inequality.
    \return <code>true</code> iff lhs and rhs have different sizes or their
            bytes are not identical.
  */
  template<typename BufferType>
  typename std::enable_if<ImplementsConcept<BufferType, Buffer>::value,
      bool>::type operator !=(const BufferType& lhs, const std::string& rhs) {
    return !(lhs == rhs);
  }

  //! Tests a Buffer and a string for equality.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff lhs and rhs have the same size and their
            bytes are identical.
  */
  template<typename BufferType>
  typename std::enable_if<ImplementsConcept<BufferType, Buffer>::value,
      bool>::type operator ==(const std::string& lhs, const BufferType& rhs) {
    return rhs == lhs;
  }

  //! Tests a Buffer and a string for inequality.
  /*!
    \param lhs The left hand side of the inequality.
    \param rhs The right hand side of the inequality.
    \return <code>true</code> iff lhs and rhs have different sizes or their
            bytes are not identical.
  */
  template<typename BufferType>
  typename std::enable_if<ImplementsConcept<BufferType, Buffer>::value,
      bool>::type operator !=(const std::string& lhs, const BufferType& rhs) {
    return !(lhs == rhs);
  }

  //! Writes a Buffer to an ostream.
  /*!
    \param stream The stream to write to.
    \param buffer The buffer to write.
    \return A reference to <code>stream</code>.
  */
  template<typename BufferType>
  typename std::enable_if<ImplementsConcept<BufferType, Buffer>::value,
      std::ostream>::type& operator <<(std::ostream& stream,
      const BufferType& buffer) {
    stream.write(buffer.GetData(), buffer.GetSize());
    return stream;
  }

  //! Encodes a Buffer to base64.
  /*!
    \param source The Buffer to encode.
    \return The base64 encoding of the <i>source</i>.
  */
  template<typename BufferType>
  std::string Base64Encode(const BufferType& source) {
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

  //! Decodes a base64 string into a Buffer.
  /*!
    \param source The string to decode.
    \param buffer The Buffer to append the data to.
  */
  template<typename BufferType>
  void Base64Decode(const std::string& source, Out<BufferType> buffer) {}
}
}

#endif
