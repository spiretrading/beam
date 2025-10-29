#ifndef BEAM_BINARY_SENDER_HPP
#define BEAM_BINARY_SENDER_HPP
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/SenderMixin.hpp"
#include "Beam/Serialization/SerializationException.hpp"
#include "Beam/Utilities/FixedString.hpp"

namespace Beam {
  template<IsConstBuffer> class BinaryReceiver;

  /**
   * Implements a Sender using a binary format.
   * @tparam S The type of Buffer to send the data to.
   */
  template<IsBuffer S>
  class BinarySender : public SenderMixin<BinarySender<S>> {
    public:
      using Sink = S;
      using SenderMixin<BinarySender>::SenderMixin;

      void set(Ref<Sink> sink);
      template<typename T> requires std::is_fundamental_v<T>
      void send(const char* name, const T& value);
      template<IsConstBuffer T>
      void send(const char* name, const T& value);
      void send(const char* name, const std::string& value);
      template<std::size_t N>
      void send(const char* name, const FixedString<N>& value);
      void start_structure(const char* name);
      void end_structure();
      void start_sequence(const char* name, const int& size);
      void start_sequence(const char* name);
      void end_sequence();
      using SenderMixin<BinarySender>::shuttle;
      using SenderMixin<BinarySender>::send;

    private:
      Sink* m_sink;
      std::size_t m_size;
  };

  template<IsBuffer S>
  void BinarySender<S>::set(Ref<Sink> sink) {
    m_sink = sink.get();
    m_size = m_sink->get_size();
  }

  template<IsBuffer S>
  template<typename T> requires std::is_fundamental_v<T>
  void BinarySender<S>::send(const char* name, const T& value) {
    append(*m_sink, value);
    m_size += sizeof(T);
  }

  template<IsBuffer S>
  template<IsConstBuffer T>
  void BinarySender<S>::send(const char* name, const T& value) {
    auto size = static_cast<std::uint32_t>(value.get_size());
    send(size);
    auto available_size = m_sink->grow(size);
    if(available_size < size) {
      boost::throw_with_location(
        SerializationException("Data length out of range."));
    }
    std::memcpy(
      get_mutable_suffix(*m_sink, available_size), value.get_data(), size);
    m_size += size;
  }

  template<IsBuffer S>
  void BinarySender<S>::send(const char* name, const std::string& value) {
    auto size = static_cast<std::uint32_t>(value.size());
    send(size);
    auto available_size = m_sink->grow(size);
    if(available_size < size) {
      boost::throw_with_location(
        SerializationException("Data length out of range."));
    }
    std::memcpy(
      get_mutable_suffix(*m_sink, available_size), value.c_str(), size);
    m_size += size;
  }

  template<IsBuffer S>
  template<std::size_t N>
  void BinarySender<S>::send(const char* name, const FixedString<N>& value) {
    auto available_size = m_sink->grow(N);
    if(available_size < N) {
      boost::throw_with_location(
        SerializationException("Data length out of range."));
    }
    std::memcpy(
      get_mutable_suffix(*m_sink, available_size), value.get_data(), N);
    m_size += N;
  }

  template<IsBuffer S>
  void BinarySender<S>::start_structure(const char* name) {}

  template<IsBuffer S>
  void BinarySender<S>::end_structure() {}

  template<IsBuffer S>
  void BinarySender<S>::start_sequence(const char* name, const int& size) {
    send(size);
  }

  template<IsBuffer S>
  void BinarySender<S>::start_sequence(const char* name) {}

  template<IsBuffer S>
  void BinarySender<S>::end_sequence() {}

  template<typename S>
  struct inverse<BinarySender<S>> {
    using type = BinaryReceiver<S>;
  };
}

#endif
