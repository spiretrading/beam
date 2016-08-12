#ifndef BEAM_JSONSENDER_HPP
#define BEAM_JSONSENDER_HPP
#include <cstring>
#include <type_traits>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/SenderMixin.hpp"
#include "Beam/Utilities/ToString.hpp"

namespace Beam {
namespace Serialization {

  /*! \class JsonSender
      \brief Implements a Sender using JSON.
      \tparam SinkType The type of Buffer to send the data to.
   */
  template<typename SinkType>
  class JsonSender : public SenderMixin<JsonSender<SinkType>> {
    public:
      static_assert(ImplementsConcept<SinkType, IO::Buffer>::value,
        "SinkType must implement the Buffer Concept.");
      typedef SinkType Sink;

      //! Constructs a JsonSender.
      JsonSender();

      //! Constructs a JsonSender.
      /*!
        \param registry The TypeRegistry used for sending polymorphic types.
      */
      JsonSender(RefType<TypeRegistry<JsonSender>> registry);

      void SetSink(RefType<Sink> sink);

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

      using SenderMixin<JsonSender<SinkType>>::Send;
      using SenderMixin<JsonSender<SinkType>>::Shuttle;

    private:
      Sink* m_sink;
      bool m_appendComma;
  };

  template<typename SinkType>
  JsonSender<SinkType>::JsonSender()
      : m_appendComma{false} {}

  template<typename SinkType>
  JsonSender<SinkType>::JsonSender(
      RefType<TypeRegistry<JsonSender>> registry)
      : SenderMixin<JsonSender<SinkType>>(Ref(registry)),
        m_appendComma{false} {}

  template<typename SinkType>
  void JsonSender<SinkType>::SetSink(RefType<Sink> sink) {
    m_appendComma = false;
    m_sink = sink.Get();
  }

  template<typename SinkType>
  template<typename T>
  typename std::enable_if<std::is_fundamental<T>::value>::type
      JsonSender<SinkType>::Send(const char* name, const T& value) {
    if(m_appendComma) {
      m_sink->Append(',');
    }
    if(name != nullptr) {
      m_sink->Append('\"');
      m_sink->Append(name, std::strlen(name));
      m_sink->Append('\"');
      m_sink->Append(':');
    }
    auto v = ToString(value);
    m_sink->Append(v.c_str(), v.size());
    m_appendComma = true;
  }

  template<typename SinkType>
  template<typename T>
  typename std::enable_if<ImplementsConcept<T, IO::Buffer>::value>::type
      JsonSender<SinkType>::Send(const char* name, const T& value) {
    if(m_appendComma) {
      m_sink->Append(',');
    }
    if(name != nullptr) {
      m_sink->Append('\"');
      m_sink->Append(name, std::strlen(name));
      m_sink->Append('\"');
      m_sink->Append(':');
    }
    auto v = std::string{"\"\""};
    m_sink->Append(v.c_str(), v.size());
    m_appendComma = true;
  }

  template<typename SinkType>
  void JsonSender<SinkType>::Send(const char* name, const std::string& value,
      unsigned int version) {
    if(m_appendComma) {
      m_sink->Append(',');
    }
    if(name != nullptr) {
      m_sink->Append('\"');
      m_sink->Append(name, std::strlen(name));
      m_sink->Append('\"');
      m_sink->Append(':');
    }
    m_sink->Append('\"');
    m_sink->Append(value.c_str(), value.size());
    m_sink->Append('\"');
    m_appendComma = true;
  }

  template<typename SinkType>
  template<std::size_t N>
  void JsonSender<SinkType>::Send(const char* name, const FixedString<N>& value,
      unsigned int version) {
    Send(name, std::string{value.GetData()});
  }

  template<typename SinkType>
  void JsonSender<SinkType>::StartStructure(const char* name) {
    if(m_appendComma) {
      m_sink->Append(',');
    }
    if(name != nullptr) {
      m_sink->Append('\"');
      m_sink->Append(name, std::strlen(name));
      m_sink->Append('\"');
      m_sink->Append(':');
    }
    m_sink->Append('{');
    m_appendComma = false;
  }

  template<typename SinkType>
  void JsonSender<SinkType>::EndStructure() {
    m_sink->Append('}');
    m_appendComma = true;
  }

  template<typename SinkType>
  void JsonSender<SinkType>::StartSequence(const char* name, const int& size) {
    StartSequence(name);
  }

  template<typename SinkType>
  void JsonSender<SinkType>::StartSequence(const char* name) {
    if(m_appendComma) {
      m_sink->Append(',');
    }
    if(name != nullptr) {
      m_sink->Append('\"');
      m_sink->Append(name, std::strlen(name));
      m_sink->Append('\"');
      m_sink->Append(':');
    }
    m_sink->Append('[');
    m_appendComma = false;
  }

  template<typename SinkType>
  void JsonSender<SinkType>::EndSequence() {
    m_sink->Append(']');
    m_appendComma = true;
  }
}

  template<typename SinkType>
  struct ImplementsConcept<Serialization::JsonSender<SinkType>,
    Serialization::Sender<SinkType>> : std::true_type {};
}

#endif
