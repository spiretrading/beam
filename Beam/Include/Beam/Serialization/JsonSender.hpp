#ifndef BEAM_JSON_SENDER_HPP
#define BEAM_JSON_SENDER_HPP
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
    auto result = std::string();
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

  /**
   * Implements a Sender using JSON.
   * @param <S> The type of Buffer to send the data to.
   */
  template<typename S>
  class JsonSender : public SenderMixin<JsonSender<S>> {
    public:
      static_assert(ImplementsConcept<S, IO::Buffer>::value,
        "Sink must implement the Buffer Concept.");
      using Sink = S;

      /** Constructs a JsonSender. */
      JsonSender();

      /**
       * Constructs a JsonSender.
       * @param registry The TypeRegistry used for sending polymorphic types.
       */
      JsonSender(Ref<const TypeRegistry<JsonSender>> registry);

      void SetSink(Ref<Sink> sink);

      void Send(const char* name, const unsigned char& value);

      void Send(const char* name, const signed char& value);

      void Send(const char* name, const char& value);

      void Send(const char* name, const bool& value);

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

      using SenderMixin<JsonSender>::Send;
      using SenderMixin<JsonSender>::Shuttle;
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

  template<typename S>
  JsonSender<S>::JsonSender()
    : m_appendComma(false) {}

  template<typename S>
  JsonSender<S>::JsonSender(Ref<const TypeRegistry<JsonSender>> registry)
    : SenderMixin<JsonSender>(Ref(registry)),
      m_appendComma(false) {}

  template<typename S>
  void JsonSender<S>::SetSink(Ref<Sink> sink) {
    m_appendComma = false;
    m_sink = sink.Get();
  }

  template<typename S>
  void JsonSender<S>::Send(const char* name, const unsigned char& value) {
    Send(name, static_cast<int>(value));
  }

  template<typename S>
  void JsonSender<S>::Send(const char* name, const signed char& value) {
    Send(name, static_cast<int>(value));
  }

  template<typename S>
  void JsonSender<S>::Send(const char* name, const char& value) {
    if(value == '\0') {
      Send(name, static_cast<int>(value));
      return;
    }
    if(m_appendComma) {
      m_sink->Append(',');
    }
    if(name) {
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

  template<typename S>
  void JsonSender<S>::Send(const char* name, const bool& value) {
    if(m_appendComma) {
      m_sink->Append(',');
    }
    if(name) {
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

  template<typename S>
  template<typename T>
  std::enable_if_t<std::is_fundamental_v<T>> JsonSender<S>::Send(
      const char* name, const T& value) {
    if(m_appendComma) {
      m_sink->Append(',');
    }
    if(name) {
      m_sink->Append('\"');
      m_sink->Append(name, std::strlen(name));
      m_sink->Append('\"');
      m_sink->Append(':');
    }
    auto v = std::to_string(value);
    m_sink->Append(v.c_str(), v.size());
    m_appendComma = true;
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<ImplementsConcept<T, IO::Buffer>::value> JsonSender<S>::Send(
      const char* name, const T& value) {
    auto base64String = IO::Base64Encode(value);
    Send(name, base64String);
  }

  template<typename S>
  void JsonSender<S>::Send(const char* name, const std::string& value,
      unsigned int version) {
    if(m_appendComma) {
      m_sink->Append(',');
    }
    if(name) {
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

  template<typename S>
  template<std::size_t N>
  void JsonSender<S>::Send(const char* name, const FixedString<N>& value,
      unsigned int version) {
    Send(name, std::string(value.GetData()));
  }

  template<typename S>
  void JsonSender<S>::StartStructure(const char* name) {
    if(m_appendComma) {
      m_sink->Append(',');
    }
    if(name) {
      m_sink->Append('\"');
      m_sink->Append(name, std::strlen(name));
      m_sink->Append('\"');
      m_sink->Append(':');
    }
    m_sink->Append('{');
    m_appendComma = false;
  }

  template<typename S>
  void JsonSender<S>::EndStructure() {
    m_sink->Append('}');
    m_appendComma = true;
  }

  template<typename S>
  void JsonSender<S>::StartSequence(const char* name, const int& size) {
    StartSequence(name);
  }

  template<typename S>
  void JsonSender<S>::StartSequence(const char* name) {
    if(m_appendComma) {
      m_sink->Append(',');
    }
    if(name) {
      m_sink->Append('\"');
      m_sink->Append(name, std::strlen(name));
      m_sink->Append('\"');
      m_sink->Append(':');
    }
    m_sink->Append('[');
    m_appendComma = false;
  }

  template<typename S>
  void JsonSender<S>::EndSequence() {
    m_sink->Append(']');
    m_appendComma = true;
  }

  template<typename S>
  struct Inverse<JsonSender<S>> {
    using type = JsonReceiver<S>;
  };
}

  template<typename S>
  struct ImplementsConcept<Serialization::JsonSender<S>,
    Serialization::Sender<S>> : std::true_type {};
}

#endif
