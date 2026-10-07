#ifndef BEAM_JSON_RECEIVER_HPP
#define BEAM_JSON_RECEIVER_HPP
#include <charconv>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <deque>
#include <optional>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Json/JsonObject.hpp"
#include "Beam/Json/JsonParser.hpp"
#include "Beam/Serialization/ReceiverMixin.hpp"
#include "Beam/Serialization/SerializationException.hpp"
#include "Beam/Utilities/FixedString.hpp"
#include "Beam/Utilities/ToString.hpp"

namespace Beam {
  template<IsBuffer> class JsonSender;

  /**
   * Implements a Receiver using the JSON format.
   * @tparam S The type of Buffer to receive the data from.
   */
  template<IsConstBuffer S>
  class JsonReceiver : public ReceiverMixin<JsonReceiver<S>> {
    public:
      using Source = S;
      using ReceiverMixin<JsonReceiver>::ReceiverMixin;

      /** Verifies that only trailing JSON whitespace remains unread. */
      void validate_end();

      void set(Ref<const Source> source);
      void receive(const char* name, bool& value);
      void receive(const char* name, unsigned char& value);
      void receive(const char* name, signed char& value);
      void receive(const char* name, char& value);
      template<typename T> requires std::is_integral_v<T>
      void receive(const char* name, T& value);
      template<typename T> requires std::is_floating_point_v<T>
      void receive(const char* name, T& value);
      template<IsBuffer T>
      void receive(const char* name, T& value);
      void receive(const char* name, std::string& value);
      void receive(const char* name, JsonValue& value);
      template<typename T>
      void receive(const char* name, std::optional<T>& value);
      template<std::size_t N>
      void receive(const char* name, FixedString<N>& value);
      void start_structure(const char* name);
      void end_structure();
      void start_sequence(const char* name, int& size);
      void start_sequence(const char* name);
      void end_sequence();
      using ReceiverMixin<JsonReceiver>::shuttle;
      using ReceiverMixin<JsonReceiver>::receive;

    private:
      struct Sequence {
        std::vector<JsonValue> m_list;
        std::size_t m_index;
      };
      boost::optional<ReaderParserStream<BufferReader<Source>>> m_parser_stream;
      using AggregateType = std::variant<JsonObject, Sequence>;
      std::deque<AggregateType> m_aggregate_queue;

      const JsonValue& extract(
        const char* name, boost::optional<JsonValue>& storage);
  };

  template<typename S>
  struct inverse<JsonReceiver<S>> {
    using type = JsonSender<S>;
  };

  /**
   * Converts JSON text to a value.
   * @tparam T The type to deserialize.
   * @param source The JSON text to deserialize.
   * @return The deserialized value.
   */
  template<typename T>
  T from_json(std::string_view source) {
    using Value = T;
    auto start = source.find_first_not_of(" \t\r\n");
    if(start != std::string_view::npos) {
      source.remove_prefix(start);
    }
    auto buffer = SharedBuffer(source.data(), source.size());
    auto receiver = JsonReceiver<SharedBuffer>();
    receiver.set(Ref(buffer));
    auto value = receive<Value>(receiver);
    receiver.validate_end();
    return value;
  }

  /**
   * Converts a JSON value to a deserialized value.
   * @tparam T The type to deserialize.
   * @param value The JSON value to deserialize.
   * @return The deserialized value.
   */
  template<typename T>
  T from_json(const std::same_as<JsonValue> auto& value) {
    using Value = T;
    return from_json<Value>(to_string(value));
  }

  template<IsConstBuffer S>
  void JsonReceiver<S>::validate_end() {
    if(!m_aggregate_queue.empty()) {
      boost::throw_with_location(
        SerializationException("Incomplete JSON value."));
    }
    while(m_parser_stream->read()) {
      auto character = m_parser_stream->peek();
      if(character != ' ' && character != '\t' && character != '\r' &&
          character != '\n') {
        boost::throw_with_location(
          SerializationException("Unexpected trailing JSON input."));
      }
    }
  }

  template<IsConstBuffer S>
  void JsonReceiver<S>::set(Ref<const Source> source) {
    m_aggregate_queue.clear();
    m_parser_stream.emplace(*source);
  }

  template<IsConstBuffer S>
  void JsonReceiver<S>::receive(const char* name, bool& value) {
    auto storage = boost::optional<JsonValue>();
    auto& json_value = extract(name, storage);
    try {
      value = std::get<bool>(json_value);
    } catch(const std::bad_variant_access&) {
      boost::throw_with_location(SerializationException("JSON type mismatch."));
    }
  }

  template<IsConstBuffer S>
  void JsonReceiver<S>::receive(const char* name, unsigned char& value) {
    auto numeric_value = int();
    receive(name, numeric_value);
    if(numeric_value < std::numeric_limits<unsigned char>::min() ||
        numeric_value > std::numeric_limits<unsigned char>::max()) {
      boost::throw_with_location(SerializationException("Value out of range."));
    }
    value = static_cast<unsigned char>(numeric_value);
  }

  template<IsConstBuffer S>
  void JsonReceiver<S>::receive(const char* name, signed char& value) {
    auto numeric_value = int();
    receive(name, numeric_value);
    if(numeric_value < std::numeric_limits<signed char>::min() ||
        numeric_value > std::numeric_limits<signed char>::max()) {
      boost::throw_with_location(SerializationException("Value out of range."));
    }
    value = static_cast<signed char>(numeric_value);
  }

  template<IsConstBuffer S>
  void JsonReceiver<S>::receive(const char* name, char& value) {
    auto storage = boost::optional<JsonValue>();
    auto& json_value = extract(name, storage);
    if(auto s = std::get_if<std::string>(&json_value)) {
      if(s->size() != 1) {
        boost::throw_with_location(
          SerializationException("Length out of range."));
      }
      value = s->front();
    } else if(std::get_if<double>(&json_value)) {
      value = '\0';
    } else {
      boost::throw_with_location(SerializationException("JSON type mismatch."));
    }
  }

  template<IsConstBuffer S>
  template<typename T> requires std::is_integral_v<T>
  void JsonReceiver<S>::receive(const char* name, T& value) {
    if constexpr(is_wide_integer<T>) {
      auto storage = boost::optional<JsonValue>();
      auto& json_value = extract(name, storage);
      if(auto s = std::get_if<std::string>(&json_value)) {
        auto result = T();
        auto end = s->data() + s->size();
        auto conversion = std::from_chars(s->data(), end, result);
        if(conversion.ec != std::errc() || conversion.ptr != end) {
          boost::throw_with_location(
            SerializationException("Value out of range."));
        }
        value = result;
      } else if(auto number = std::get_if<double>(&json_value)) {
        value = static_cast<T>(*number);
      } else {
        boost::throw_with_location(
          SerializationException("JSON type mismatch."));
      }
    } else {
      auto raw_value = double();
      receive(name, raw_value);
      value = static_cast<T>(raw_value);
    }
  }

  template<IsConstBuffer S>
  template<typename T> requires std::is_floating_point_v<T>
  void JsonReceiver<S>::receive(const char* name, T& value) {
    auto storage = boost::optional<JsonValue>();
    auto& json_value = extract(name, storage);
    if(auto s = std::get_if<double>(&json_value)) {
      value = static_cast<T>(*s);
    } else {
      boost::throw_with_location(SerializationException("JSON type mismatch."));
    }
  }

  template<IsConstBuffer S>
  template<IsBuffer T>
  void JsonReceiver<S>::receive(const char* name, T& value) {
    auto base64 = std::string();
    receive(name, base64);
    decode_base64(base64, out(value));
  }

  template<IsConstBuffer S>
  void JsonReceiver<S>::receive(const char* name, std::string& value) {
    auto storage = boost::optional<JsonValue>();
    auto& json_value = extract(name, storage);
    if(auto s = std::get_if<std::string>(&json_value)) {
      value = std::move(*s);
    } else {
      boost::throw_with_location(SerializationException("JSON type mismatch."));
    }
  }

  template<IsConstBuffer S>
  void JsonReceiver<S>::receive(const char* name, JsonValue& value) {
    auto storage = boost::optional<JsonValue>();
    value = extract(name, storage);
  }

  template<IsConstBuffer S>
  template<typename T>
  void JsonReceiver<S>::receive(const char* name, std::optional<T>& value) {
    auto field = false;
    if(name && !m_aggregate_queue.empty()) {
      if(auto object = std::get_if<JsonObject>(&m_aggregate_queue.back())) {
        if(!object->get(name)) {
          value.reset();
          return;
        }
        field = true;
      }
    }
    auto storage = boost::optional<JsonValue>();
    auto& json_value = extract(name, storage);
    if(std::get_if<JsonNull>(&json_value) &&
        (!field || !std::is_same_v<T, JsonValue>)) {
      value.reset();
      return;
    }
    auto object = JsonObject();
    object.set("value", json_value);
    m_aggregate_queue.push_back(object);
    try {
      value.emplace(Beam::receive<T>(*this, "value"));
    } catch(...) {
      m_aggregate_queue.pop_back();
      throw;
    }
    m_aggregate_queue.pop_back();
  }

  template<IsConstBuffer S>
  template<std::size_t N>
  void JsonReceiver<S>::receive(const char* name, FixedString<N>& value) {
    auto storage = boost::optional<JsonValue>();
    auto& json_value = extract(name, storage);
    if(auto s = std::get_if<std::string>(&json_value)) {
      if(s->size() > N) {
        boost::throw_with_location(
          SerializationException("Length out of range."));
      }
      value = std::move(*s);
    } else {
      boost::throw_with_location(SerializationException("JSON type mismatch."));
    }
  }

  template<IsConstBuffer S>
  void JsonReceiver<S>::start_structure(const char* name) {
    auto storage = boost::optional<JsonValue>();
    auto& json_value = extract(name, storage);
    if(auto s = std::get_if<JsonObject>(&json_value)) {
      if(!s->get("__version")) {
        const_cast<JsonObject&>(*s).set("__version", 0.0);
      }
      m_aggregate_queue.push_back(*s);
    } else {
      boost::throw_with_location(SerializationException("JSON type mismatch."));
    }
  }

  template<IsConstBuffer S>
  void JsonReceiver<S>::end_structure() {
    m_aggregate_queue.pop_back();
  }

  template<IsConstBuffer S>
  void JsonReceiver<S>::start_sequence(const char* name, int& size) {
    auto storage = boost::optional<JsonValue>();
    auto& json_value = extract(name, storage);
    if(auto s = std::get_if<std::vector<JsonValue>>(&json_value)) {
      auto sequence = Sequence();
      sequence.m_list = std::move(*s);
      sequence.m_index = 0;
      size = static_cast<int>(sequence.m_list.size());
      m_aggregate_queue.push_back(std::move(sequence));
    } else {
      boost::throw_with_location(SerializationException("JSON type mismatch."));
    }
  }

  template<IsConstBuffer S>
  void JsonReceiver<S>::start_sequence(const char* name) {
    auto dummy = int();
    start_sequence(name, dummy);
  }

  template<IsConstBuffer S>
  void JsonReceiver<S>::end_sequence() {
    m_aggregate_queue.pop_back();
  }

  template<IsConstBuffer S>
  const JsonValue& JsonReceiver<S>::extract(
      const char* name, boost::optional<JsonValue>& storage) {
    if(m_aggregate_queue.empty()) {
      storage.emplace();
      if(!json_p.read(*m_parser_stream, *storage)) {
        boost::throw_with_location(
          SerializationException("Invalid JSON format."));
      }
      return *storage;
    } else if(auto aggregate =
        std::get_if<JsonObject>(&m_aggregate_queue.back()))  {
      if(name) {
        return aggregate->at(name);
      } else {
        boost::throw_with_location(
          SerializationException("Invalid JSON format."));
      }
    } else if(auto aggregate =
        std::get_if<Sequence>(&m_aggregate_queue.back())) {
      if(aggregate->m_index >= aggregate->m_list.size()) {
        boost::throw_with_location(
          SerializationException("JSON sequence out of range."));
      }
      auto& value = aggregate->m_list[aggregate->m_index];
      ++aggregate->m_index;
      return value;
    }
    boost::throw_with_location(SerializationException("Invalid JSON format."));
  }
}

#endif
