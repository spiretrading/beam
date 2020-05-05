#ifndef BEAM_JSONSENDER_HPP
#define BEAM_JSONSENDER_HPP
#include <cstring>
#include <string>
#include <type_traits>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/SenderMixin.hpp"

namespace Beam {
namespace Serialization {
namespace Details {
  inline std::string JsonEscape(const std::string& source) {
    std::string result;
    for(auto c : source) {
      if(c == '\\') {
        result += "\\\\";
      } else if(c == '\n') {
        result += "\\n";
      } else if(c == '\r') {
        result += "\\r";
      } else if(c == '\"') {
        result += "\\\"";
      } else if(c == '\b') {
        result += "\\b";
      } else if(c == '\f') {
        result += "\\f";
      } else if(c == '\t') {
        result += "\\t";
      } else {
        result += c;
      }
    }
    return result;
  }
}

  /*! \class JsonSender
      \brief Implements a Sender using JSON.
      \tparam SinkType The type of Buffer to send the data to.
   */
  template<typename SinkType>
  class JsonSender : public SenderMixin<JsonSender<SinkType>> {
    public:
      static_assert(ImplementsConcept<SinkType, IO::Buffer>::value,
        "SinkType must implement the Buffer Concept.");
      using Sink = SinkType;

      //! Constructs a JsonSender.
      JsonSender();

      //! Constructs a JsonSender.
      /*!
        \param registry The TypeRegistry used for sending polymorphic types.
      */
      JsonSender(Ref<TypeRegistry<JsonSender>> registry);

      void SetSink(Ref<Sink> sink);

      void Send(const char* name, const unsigned char& value);

      void Send(const char* name, const signed char& value);

      void Send(const char* name, const char& value);

      void Send(const char* name, const bool& value);

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

  /** Converts an object to its JSON representation. */
  template<typename T>
  std::string ToJson(const T& object) {
    auto sender = JsonSender<IO::SharedBuffer>();
    auto buffer = IO::SharedBuffer();
    sender.SetSink(Ref(buffer));
    sender.Send(object);
    return std::string(buffer.GetData(), buffer.GetSize());
  }

  template<typename SinkType>
  JsonSender<SinkType>::JsonSender()
      : m_appendComma{false} {}

  template<typename SinkType>
  JsonSender<SinkType>::JsonSender(Ref<TypeRegistry<JsonSender>> registry)
      : SenderMixin<JsonSender<SinkType>>(Ref(registry)),
        m_appendComma{false} {}

  template<typename SinkType>
  void JsonSender<SinkType>::SetSink(Ref<Sink> sink) {
    m_appendComma = false;
    m_sink = sink.Get();
  }

  template<typename SinkType>
  void JsonSender<SinkType>::Send(const char* name,
      const unsigned char& value) {
    Send(name, static_cast<int>(value));
  }

  template<typename SinkType>
  void JsonSender<SinkType>::Send(const char* name, const signed char& value) {
    Send(name, static_cast<int>(value));
  }

  template<typename SinkType>
  void JsonSender<SinkType>::Send(const char* name, const char& value) {
    if(value == '\0') {
      Send(name, static_cast<int>(value));
      return;
    }
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
    m_sink->Append(value);
    m_sink->Append('\"');
    m_appendComma = true;
  }

  template<typename SinkType>
  void JsonSender<SinkType>::Send(const char* name, const bool& value) {
    if(m_appendComma) {
      m_sink->Append(',');
    }
    if(name != nullptr) {
      m_sink->Append('\"');
      m_sink->Append(name, std::strlen(name));
      m_sink->Append('\"');
      m_sink->Append(':');
    }
    if(value) {
      m_sink->Append("true", 4);
    } else {
      m_sink->Append("false", 5);
    }
    m_appendComma = true;
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
    auto v = std::to_string(value);
    m_sink->Append(v.c_str(), v.size());
    m_appendComma = true;
  }

  template<typename SinkType>
  template<typename T>
  typename std::enable_if<ImplementsConcept<T, IO::Buffer>::value>::type
      JsonSender<SinkType>::Send(const char* name, const T& value) {
    auto base64String = IO::Base64Encode(value);
    Send(name, base64String);
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
    auto escapedValue = Details::JsonEscape(value);
    m_sink->Append(escapedValue.c_str(), escapedValue.size());
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

  template<typename SinkType>
  struct Inverse<JsonSender<SinkType>> {
    using type = JsonReceiver<SinkType>;
  };
}

  template<typename SinkType>
  struct ImplementsConcept<Serialization::JsonSender<SinkType>,
    Serialization::Sender<SinkType>> : std::true_type {};
}

#endif
