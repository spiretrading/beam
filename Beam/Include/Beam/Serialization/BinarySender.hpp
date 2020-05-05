#ifndef BEAM_BINARYSENDER_HPP
#define BEAM_BINARYSENDER_HPP
#include <cstdint>
#include <cstring>
#include <type_traits>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/SenderMixin.hpp"
#include "Beam/Utilities/FixedString.hpp"

namespace Beam {
namespace Serialization {

  /*! \class BinarySender
      \brief Implements a Sender using a binary format.
      \tparam SinkType The type of Buffer to send the data to.
   */
  template<typename SinkType>
  class BinarySender : public SenderMixin<BinarySender<SinkType>> {
    public:
      static_assert(ImplementsConcept<SinkType, IO::Buffer>::value,
        "SinkType must implement the Buffer Concept.");
      using Sink = SinkType;

      //! Constructs a BinarySender.
      BinarySender() = default;

      //! Constructs a BinarySender.
      /*!
        \param registry The TypeRegistry used for sending polymorphic types.
      */
      BinarySender(Ref<TypeRegistry<BinarySender>> registry);

      void SetSink(Ref<Sink> sink);

      template<typename T>
      typename std::enable_if<std::is_fundamental<T>::value>::type Send(
        const char* name, const T& value);

      template<typename T>
      typename std::enable_if<ImplementsConcept<T, IO::Buffer>::value>::type
        Send(const char* name, const T& value);

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

      using SenderMixin<BinarySender<SinkType>>::Send;
      using SenderMixin<BinarySender<SinkType>>::Shuttle;

    private:
      Sink* m_sink;
      std::size_t m_size;
  };

  template<typename SinkType>
  BinarySender<SinkType>::BinarySender(
      Ref<TypeRegistry<BinarySender>> registry)
      : SenderMixin<BinarySender<SinkType>>(Ref(registry)) {}

  template<typename SinkType>
  void BinarySender<SinkType>::SetSink(Ref<Sink> sink) {
    m_sink = sink.Get();
    m_size = m_sink->GetSize();
  }

  template<typename SinkType>
  template<typename T>
  typename std::enable_if<std::is_fundamental<T>::value>::type
      BinarySender<SinkType>::Send(const char* name, const T& value) {
    m_sink->Grow(sizeof(T));
    std::memcpy(m_sink->GetMutableData() + m_size,
      reinterpret_cast<const char*>(&value), sizeof(T));
    m_size += sizeof(T);
  }

  template<typename SinkType>
  template<typename T>
  typename std::enable_if<ImplementsConcept<T, IO::Buffer>::value>::type
      BinarySender<SinkType>::Send(const char* name, const T& value) {
    auto size = static_cast<std::uint32_t>(value.GetSize());
    Shuttle(size);
    m_sink->Grow(size);
    std::memcpy(m_sink->GetMutableData() + m_size, value.GetData(), size);
    m_size += size;
  }

  template<typename SinkType>
  void BinarySender<SinkType>::Send(const char* name, const std::string& value,
      unsigned int version) {
    auto size = static_cast<std::uint32_t>(value.size());
    Shuttle(size);
    m_sink->Grow(size);
    std::memcpy(m_sink->GetMutableData() + m_size, value.c_str(), size);
    m_size += size;
  }

  template<typename SinkType>
  template<std::size_t N>
  void BinarySender<SinkType>::Send(const char* name,
      const FixedString<N>& value, unsigned int version) {
    m_sink->Grow(N);
    std::memcpy(m_sink->GetMutableData() + m_size, value.GetData(), N);
    m_size += N;
  }

  template<typename SinkType>
  void BinarySender<SinkType>::StartStructure(const char* name) {}

  template<typename SinkType>
  void BinarySender<SinkType>::EndStructure() {}

  template<typename SinkType>
  void BinarySender<SinkType>::StartSequence(const char* name,
      const int& size) {
    Shuttle(size);
  }

  template<typename SinkType>
  void BinarySender<SinkType>::StartSequence(const char* name) {}

  template<typename SinkType>
  void BinarySender<SinkType>::EndSequence() {}

  template<typename SinkType>
  struct Inverse<BinarySender<SinkType>> {
    using type = BinaryReceiver<SinkType>;
  };
}

  template<typename SinkType>
  struct ImplementsConcept<Serialization::BinarySender<SinkType>,
    Serialization::Sender<SinkType>> : std::true_type {};
}

#endif
