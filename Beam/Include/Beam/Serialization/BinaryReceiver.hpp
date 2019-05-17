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

  /** Implements a Receiver using a binary format.
      \tparam SourceType The type of Buffer to receive the data from.
   */
  template<typename SourceType>
  class BinaryReceiver : public ReceiverMixin<BinaryReceiver<SourceType>> {
    public:
      static_assert(ImplementsConcept<SourceType, IO::Buffer>::value,
        "SourceType must implement the Buffer Concept.");
      using Source = SourceType;

      //! Constructs a BinaryReceiver.
      BinaryReceiver() = default;

      //! Constructs a BinaryReceiver.
      /*!
        \param registry The TypeRegistry used for receiving polymorphic types.
      */
      BinaryReceiver(Ref<TypeRegistry<BinarySender<SourceType>>> registry);

      void SetSource(Ref<const Source> source);

      template<typename T>
      typename std::enable_if<std::is_fundamental<T>::value>::type Shuttle(
        const char* name, T& value);

      template<typename T>
      typename std::enable_if<ImplementsConcept<T, IO::Buffer>::value>::type
        Shuttle(const char* name, T& value);

      void Shuttle(const char* name, std::string& value);

      template<std::size_t N>
      void Shuttle(const char* name, FixedString<N>& value);

      void StartStructure(const char* name);

      void EndStructure();

      void StartSequence(const char* name, int& size);

      void StartSequence(const char* name);

      void EndSequence();

      using ReceiverMixin<BinaryReceiver<SourceType>>::Shuttle;

    private:
      std::size_t m_remainingSize;
      const char* m_readIterator;
  };

  template<typename SourceType>
  BinaryReceiver<SourceType>::BinaryReceiver(Ref<TypeRegistry<
      BinarySender<SourceType>>> registry)
      : ReceiverMixin<BinaryReceiver<SourceType>>(Ref(registry)) {}

  template<typename SourceType>
  void BinaryReceiver<SourceType>::SetSource(Ref<const Source> source) {
    m_remainingSize = source->GetSize();
    m_readIterator = source->GetData();
  }

  template<typename SourceType>
  template<typename T>
  typename std::enable_if<std::is_fundamental<T>::value>::type
      BinaryReceiver<SourceType>::Shuttle(const char* name, T& value) {
    if(sizeof(T) > m_remainingSize) {
      BOOST_THROW_EXCEPTION(SerializationException(
        "Data length out of range."));
    }
    std::memcpy(reinterpret_cast<char*>(&value), m_readIterator, sizeof(T));
    m_readIterator += sizeof(T);
    m_remainingSize -= sizeof(T);
  }

  template<typename SourceType>
  template<typename T>
  typename std::enable_if<ImplementsConcept<T, IO::Buffer>::value>::type
      BinaryReceiver<SourceType>::Shuttle(const char* name, T& value) {
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

  template<typename SourceType>
  void BinaryReceiver<SourceType>::Shuttle(const char* name,
      std::string& value) {
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

  template<typename SourceType>
  template<std::size_t N>
  void BinaryReceiver<SourceType>::Shuttle(const char* name,
      FixedString<N>& value) {
    if(N > m_remainingSize) {
      BOOST_THROW_EXCEPTION(SerializationException(
        "String length out of range."));
    }
    value = FixedString<N>(m_readIterator, N);
    m_readIterator += N;
    m_remainingSize -= N;
  }

  template<typename SourceType>
  void BinaryReceiver<SourceType>::StartStructure(const char* name) {}

  template<typename SourceType>
  void BinaryReceiver<SourceType>::EndStructure() {}

  template<typename SourceType>
  void BinaryReceiver<SourceType>::StartSequence(const char* name, int& size) {
    Shuttle(size);
  }

  template<typename SourceType>
  void BinaryReceiver<SourceType>::StartSequence(const char* name) {}

  template<typename SourceType>
  void BinaryReceiver<SourceType>::EndSequence() {}

  template<typename SourceType>
  struct Inverse<BinaryReceiver<SourceType>> {
    using type = BinarySender<SourceType>;
  };
}

  template<typename SourceType>
  struct ImplementsConcept<Serialization::BinaryReceiver<SourceType>,
    Serialization::Receiver<SourceType>> : std::true_type {};
}

#endif
