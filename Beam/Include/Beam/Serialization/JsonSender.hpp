#ifndef BEAM_JSON_SENDER_HPP
#define BEAM_JSON_SENDER_HPP
#include <cstring>
#include <string>
#include <type_traits>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/SenderMixin.hpp"
#include "Beam/Utilities/FixedString.hpp"

namespace Beam {
namespace Details {
  inline std::string escape_json(const std::string& source) {
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
  template<IsConstBuffer> class JsonReceiver;

  /**
   * Implements a Sender using JSON.
   * @tparam S The type of Buffer to send the data to.
   */
  template<IsBuffer S>
  class JsonSender : public SenderMixin<JsonSender<S>> {
    public:
      using Sink = S;

      /** Constructs a JsonSender. */
      JsonSender();

      /**
       * Constructs a JsonSender.
       * @param registry The TypeRegistry used for sending polymorphic types.
       */
      JsonSender(Ref<const TypeRegistry<JsonSender>> registry);

      void set(Ref<Sink> sink);
      void send(const char* name, const unsigned char& value);
      void send(const char* name, const signed char& value);
      void send(const char* name, const char& value);
      void send(const char* name, const bool& value);
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
      using SenderMixin<JsonSender>::shuttle;
      using SenderMixin<JsonSender>::send;

    private:
      Sink* m_sink;
      bool m_append_comma;
  };

  template<typename S>
  struct inverse<JsonSender<S>> {
    using type = JsonReceiver<S>;
  };

  /** Converts an object to its JSON representation. */
  template<typename T>
  std::string to_json(const T& object) {
    auto sender = JsonSender<SharedBuffer>();
    auto buffer = SharedBuffer();
    sender.set(Ref(buffer));
    sender.send(object);
    return std::string(buffer.get_data(), buffer.get_size());
  }

  template<IsBuffer S>
  JsonSender<S>::JsonSender()
    : m_append_comma(false) {}

  template<IsBuffer S>
  JsonSender<S>::JsonSender(Ref<const TypeRegistry<JsonSender>> registry)
    : SenderMixin<JsonSender>(Ref(registry)),
      m_append_comma(false) {}

  template<IsBuffer S>
  void JsonSender<S>::set(Ref<Sink> sink) {
    m_append_comma = false;
    m_sink = sink.get();
  }

  template<IsBuffer S>
  void JsonSender<S>::send(const char* name, const unsigned char& value) {
    send(name, static_cast<int>(value));
  }

  template<IsBuffer S>
  void JsonSender<S>::send(const char* name, const signed char& value) {
    send(name, static_cast<int>(value));
  }

  template<IsBuffer S>
  void JsonSender<S>::send(const char* name, const char& value) {
    if(value == '\0') {
      send(name, static_cast<int>(value));
      return;
    }
    if(m_append_comma) {
      append(*m_sink, ',');
    }
    if(name) {
      append(*m_sink, '\"');
      append(*m_sink, name, std::strlen(name));
      append(*m_sink, '\"');
      append(*m_sink, ':');
    }
    append(*m_sink, '\"');
    append(*m_sink, value);
    append(*m_sink, '\"');
    m_append_comma = true;
  }

  template<IsBuffer S>
  void JsonSender<S>::send(const char* name, const bool& value) {
    if(m_append_comma) {
      append(*m_sink, ',');
    }
    if(name) {
      append(*m_sink, '\"');
      append(*m_sink, name, std::strlen(name));
      append(*m_sink, '\"');
      append(*m_sink, ':');
    }
    if(value) {
      append(*m_sink, "true", 4);
    } else {
      append(*m_sink, "false", 5);
    }
    m_append_comma = true;
  }

  template<IsBuffer S>
  template<typename T> requires std::is_fundamental_v<T>
  void JsonSender<S>::send(const char* name, const T& value) {
    if(m_append_comma) {
      append(*m_sink, ',');
    }
    if(name) {
      append(*m_sink, '\"');
      append(*m_sink, name, std::strlen(name));
      append(*m_sink, '\"');
      append(*m_sink, ':');
    }
    auto v = std::to_string(value);
    append(*m_sink, v.c_str(), v.size());
    m_append_comma = true;
  }

  template<IsBuffer S>
  template<IsConstBuffer T>
  void JsonSender<S>::send(const char* name, const T& value) {
    auto base64 = encode_base64(value);
    send(name, base64);
  }

  template<IsBuffer S>
  void JsonSender<S>::send(const char* name, const std::string& value) {
    if(m_append_comma) {
      append(*m_sink, ',');
    }
    if(name) {
      append(*m_sink, '\"');
      append(*m_sink, name, std::strlen(name));
      append(*m_sink, '\"');
      append(*m_sink, ':');
    }
    append(*m_sink, '\"');
    auto escaped_value = Details::escape_json(value);
    append(*m_sink, escaped_value.c_str(), escaped_value.size());
    append(*m_sink, '\"');
    m_append_comma = true;
  }

  template<IsBuffer S>
  template<std::size_t N>
  void JsonSender<S>::send(const char* name, const FixedString<N>& value) {
    send(name, std::string(value.get_data()));
  }

  template<IsBuffer S>
  void JsonSender<S>::start_structure(const char* name) {
    if(m_append_comma) {
      append(*m_sink, ',');
    }
    if(name) {
      append(*m_sink, '\"');
      append(*m_sink, name, std::strlen(name));
      append(*m_sink, '\"');
      append(*m_sink, ':');
    }
    append(*m_sink, '{');
    m_append_comma = false;
  }

  template<IsBuffer S>
  void JsonSender<S>::end_structure() {
    append(*m_sink, '}');
    m_append_comma = true;
  }

  template<IsBuffer S>
  void JsonSender<S>::start_sequence(const char* name, const int& size) {
    start_sequence(name);
  }

  template<IsBuffer S>
  void JsonSender<S>::start_sequence(const char* name) {
    if(m_append_comma) {
      append(*m_sink, ',');
    }
    if(name) {
      append(*m_sink, '\"');
      append(*m_sink, name, std::strlen(name));
      append(*m_sink, '\"');
      append(*m_sink, ':');
    }
    append(*m_sink, '[');
    m_append_comma = false;
  }

  template<IsBuffer S>
  void JsonSender<S>::end_sequence() {
    append(*m_sink, ']');
    m_append_comma = true;
  }
}

#endif
