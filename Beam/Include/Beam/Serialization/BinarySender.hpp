#ifndef BEAM_BINARY_SENDER_HPP
#define BEAM_BINARY_SENDER_HPP
#include <cstdint>
#include <cstring>
#include <type_traits>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/SenderMixin.hpp"
#include "Beam/Utilities/FixedString.hpp"

namespace Beam {
namespace Serialization {

  /**
   * Implements a Sender using a binary format.
   * @param <S> The type of Buffer to send the data to.
   */
  template<typename S>
  class BinarySender : public SenderMixin<BinarySender<S>> {
    public:
      static_assert(ImplementsConcept<S, IO::Buffer>::value,
        "Sink must implement the Buffer Concept.");
      using Sink = S;
      using SenderMixin<BinarySender>::SenderMixin;

      void SetSink(Ref<Sink> sink);

      template<typename T>
      std::enable_if_t<std::is_fundamental_v<T>> Send(
        const char* name, const T& value);

      template<typename T>
      std::enable_if_t<ImplementsConcept<T, IO::Buffer>::value> Send(
        const char* name, const T& value);

      void Send(const char* name, const std::string& value,
        unsigned int version);

      template<std::size_t N>
      void Send(const char* name, const FixedString<N>& value,
        unsigned int version);

      void StartStructure(const char* name);

      void EndStructure();

      void StartSequence(const char* name, const int& size);

      void StartSequence(const char* name);

      void EndSequence();

      using SenderMixin<BinarySender>::Send;
      using SenderMixin<BinarySender>::Shuttle;

    private:
      Sink* m_sink;
      std::size_t m_size;
  };

  template<typename S>
  void BinarySender<S>::SetSink(Ref<Sink> sink) {
    m_sink = sink.Get();
    m_size = m_sink->GetSize();
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<std::is_fundamental_v<T>> BinarySender<S>::Send(
      const char* name, const T& value) {
    m_sink->Grow(sizeof(T));
    std::memcpy(m_sink->GetMutableData() + m_size,
      reinterpret_cast<const char*>(&value), sizeof(T));
    m_size += sizeof(T);
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<ImplementsConcept<T, IO::Buffer>::value>
      BinarySender<S>::Send(const char* name, const T& value) {
    auto size = static_cast<std::uint32_t>(value.GetSize());
    Shuttle(size);
    m_sink->Grow(size);
    std::memcpy(m_sink->GetMutableData() + m_size, value.GetData(), size);
    m_size += size;
  }

  template<typename S>
  void BinarySender<S>::Send(const char* name, const std::string& value,
      unsigned int version) {
    auto size = static_cast<std::uint32_t>(value.size());
    Shuttle(size);
    m_sink->Grow(size);
    std::memcpy(m_sink->GetMutableData() + m_size, value.c_str(), size);
    m_size += size;
  }

  template<typename S>
  template<std::size_t N>
  void BinarySender<S>::Send(const char* name, const FixedString<N>& value,
      unsigned int version) {
    m_sink->Grow(N);
    std::memcpy(m_sink->GetMutableData() + m_size, value.GetData(), N);
    m_size += N;
  }

  template<typename S>
  void BinarySender<S>::StartStructure(const char* name) {}

  template<typename S>
  void BinarySender<S>::EndStructure() {}

  template<typename S>
  void BinarySender<S>::StartSequence(const char* name, const int& size) {
    Shuttle(size);
  }

  template<typename S>
  void BinarySender<S>::StartSequence(const char* name) {}

  template<typename S>
  void BinarySender<S>::EndSequence() {}

  template<typename S>
  struct Inverse<BinarySender<S>> {
    using type = BinaryReceiver<S>;
  };
}

  template<typename S>
  struct ImplementsConcept<Serialization::BinarySender<S>,
    Serialization::Sender<S>> : std::true_type {};
}

#endif
