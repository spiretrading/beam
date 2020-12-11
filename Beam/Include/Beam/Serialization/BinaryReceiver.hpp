#ifndef BEAM_BINARY_RECEIVER_HPP
#define BEAM_BINARY_RECEIVER_HPP
#include <cstdint>
#include <cstring>
#include <type_traits>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/ReceiverMixin.hpp"
#include "Beam/Serialization/SerializationException.hpp"
#include "Beam/Utilities/FixedString.hpp"

namespace Beam {
namespace Serialization {

  /**
   * Implements a Receiver using a binary format.
   * @param <S> The type of Buffer to receive the data from.
   */
  template<typename S>
  class BinaryReceiver : public ReceiverMixin<BinaryReceiver<S>> {
    public:
      static_assert(ImplementsConcept<S, IO::Buffer>::value,
        "S must implement the Buffer Concept.");
      using Source = S;
      using ReceiverMixin<BinaryReceiver<S>>::ReceiverMixin;

      void SetSource(Ref<const Source> source);

      template<typename T>
      std::enable_if_t<std::is_fundamental_v<T>> Shuttle(const char* name,
        T& value);

      template<typename T>
      std::enable_if_t<ImplementsConcept<T, IO::Buffer>::value> Shuttle(
        const char* name, T& value);

      void Shuttle(const char* name, std::string& value);

      template<std::size_t N>
      void Shuttle(const char* name, FixedString<N>& value);

      void StartStructure(const char* name);

      void EndStructure();

      void StartSequence(const char* name, int& size);

      void StartSequence(const char* name);

      void EndSequence();

      using ReceiverMixin<BinaryReceiver>::Shuttle;
    private:
      std::size_t m_remainingSize;
      const char* m_readIterator;
  };

  template<typename S>
  void BinaryReceiver<S>::SetSource(Ref<const Source> source) {
    m_remainingSize = source->GetSize();
    m_readIterator = source->GetData();
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<std::is_fundamental_v<T>> BinaryReceiver<S>::Shuttle(
      const char* name, T& value) {
    if(sizeof(T) > m_remainingSize) {
      BOOST_THROW_EXCEPTION(SerializationException(
        "Data length out of range."));
    }
    std::memcpy(reinterpret_cast<char*>(&value), m_readIterator, sizeof(T));
    m_readIterator += sizeof(T);
    m_remainingSize -= sizeof(T);
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<ImplementsConcept<T, IO::Buffer>::value>
      BinaryReceiver<S>::Shuttle(const char* name, T& value) {
    auto size = std::uint32_t();
    Shuttle(size);
    if(size < 0 || size > m_remainingSize) {
      BOOST_THROW_EXCEPTION(SerializationException(
        "Buffer length out of range."));
    }
    value.Reset();
    value.Append(m_readIterator, size);
    m_readIterator += size;
    m_remainingSize -= size;
  }

  template<typename S>
  void BinaryReceiver<S>::Shuttle(const char* name, std::string& value) {
    auto size = std::uint32_t();
    Shuttle(size);
    if(size < 0 || size > m_remainingSize) {
      BOOST_THROW_EXCEPTION(SerializationException(
        "String length out of range."));
    }
    value = std::string(m_readIterator, size);
    m_readIterator += size;
    m_remainingSize -= size;
  }

  template<typename S>
  template<std::size_t N>
  void BinaryReceiver<S>::Shuttle(const char* name, FixedString<N>& value) {
    if(N > m_remainingSize) {
      BOOST_THROW_EXCEPTION(SerializationException(
        "String length out of range."));
    }
    value = FixedString<N>(m_readIterator, N);
    m_readIterator += N;
    m_remainingSize -= N;
  }

  template<typename S>
  void BinaryReceiver<S>::StartStructure(const char* name) {}

  template<typename S>
  void BinaryReceiver<S>::EndStructure() {}

  template<typename S>
  void BinaryReceiver<S>::StartSequence(const char* name, int& size) {
    Shuttle(size);
  }

  template<typename S>
  void BinaryReceiver<S>::StartSequence(const char* name) {}

  template<typename S>
  void BinaryReceiver<S>::EndSequence() {}

  template<typename S>
  struct Inverse<BinaryReceiver<S>> {
    using type = BinarySender<S>;
  };
}

  template<typename S>
  struct ImplementsConcept<Serialization::BinaryReceiver<S>,
    Serialization::Receiver<S>> : std::true_type {};
}

#endif
