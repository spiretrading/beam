#ifndef BEAM_BINARY_RECEIVER_HPP
#define BEAM_BINARY_RECEIVER_HPP
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/ReceiverMixin.hpp"
#include "Beam/Serialization/SerializationException.hpp"
#include "Beam/Utilities/FixedString.hpp"

namespace Beam {
  template<IsBuffer> class BinarySender;

  /**
   * Implements a Receiver using a binary format.
   * @tparam S The type of Buffer to receive the data from.
   */
  template<IsConstBuffer S>
  class BinaryReceiver : public ReceiverMixin<BinaryReceiver<S>> {
    public:
      using Source = S;
      using ReceiverMixin<BinaryReceiver>::ReceiverMixin;

      void set(Ref<const Source> source);
      template<typename T> requires std::is_fundamental_v<T>
      void receive(const char* name, T& value);
      template<IsBuffer T>
      void receive(const char* name, T& value);
      void receive(const char* name, std::string& value);
      template<std::size_t N>
      void receive(const char* name, FixedString<N>& value);
      void start_structure(const char* name);
      void end_structure();
      void start_sequence(const char* name, int& size);
      void start_sequence(const char* name);
      void end_sequence();
      using ReceiverMixin<BinaryReceiver>::shuttle;
      using ReceiverMixin<BinaryReceiver>::receive;

    private:
      std::size_t m_remaining_size;
      const char* m_cursor;
  };

  template<IsConstBuffer S>
  void BinaryReceiver<S>::set(Ref<const Source> source) {
    m_remaining_size = source->get_size();
    m_cursor = source->get_data();
  }

  template<IsConstBuffer S>
  template<typename T> requires std::is_fundamental_v<T>
  void BinaryReceiver<S>::receive(const char* name, T& value) {
    if(sizeof(T) > m_remaining_size) {
      boost::throw_with_location(
        SerializationException("Data length out of range."));
    }
    std::memcpy(reinterpret_cast<char*>(&value), m_cursor, sizeof(T));
    m_cursor += sizeof(T);
    m_remaining_size -= sizeof(T);
  }

  template<IsConstBuffer S>
  template<IsBuffer T>
  void BinaryReceiver<S>::receive(const char* name, T& value) {
    auto size = std::uint32_t();
    receive(size);
    if(size > m_remaining_size) {
      boost::throw_with_location(
        SerializationException("Buffer length out of range."));
    }
    reset(value);
    append(value, m_cursor, size);
    m_cursor += size;
    m_remaining_size -= size;
  }

  template<IsConstBuffer S>
  void BinaryReceiver<S>::receive(const char* name, std::string& value) {
    auto size = std::uint32_t();
    receive(size);
    if(size > m_remaining_size) {
      boost::throw_with_location(
        SerializationException("String length out of range."));
    }
    value.assign(m_cursor, m_cursor + size);
    m_cursor += size;
    m_remaining_size -= size;
  }

  template<IsConstBuffer S>
  template<std::size_t N>
  void BinaryReceiver<S>::receive(const char* name, FixedString<N>& value) {
    if(N > m_remaining_size) {
      boost::throw_with_location(
        SerializationException("String length out of range."));
    }
    value = FixedString<N>(std::string_view(m_cursor, N));
    m_cursor += N;
    m_remaining_size -= N;
  }

  template<IsConstBuffer S>
  void BinaryReceiver<S>::start_structure(const char* name) {}

  template<IsConstBuffer S>
  void BinaryReceiver<S>::end_structure() {}

  template<IsConstBuffer S>
  void BinaryReceiver<S>::start_sequence(const char* name, int& size) {
    receive(size);
  }

  template<IsConstBuffer S>
  void BinaryReceiver<S>::start_sequence(const char* name) {}

  template<IsConstBuffer S>
  void BinaryReceiver<S>::end_sequence() {}

  template<typename S>
  struct inverse<BinaryReceiver<S>> {
    using type = BinarySender<S>;
  };
}

#endif
