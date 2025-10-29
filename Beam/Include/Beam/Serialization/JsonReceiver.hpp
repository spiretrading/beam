#ifndef BEAM_JSON_RECEIVER_HPP
#define BEAM_JSON_RECEIVER_HPP
#include <cstdint>
#include <cstring>
#include <deque>
#include <type_traits>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Json/JsonParser.hpp"
#include "Beam/Serialization/ReceiverMixin.hpp"
#include "Beam/Serialization/SerializationException.hpp"
#include "Beam/Utilities/FixedString.hpp"

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
      using AggregateType = boost::variant<JsonObject, Sequence>;
      std::deque<AggregateType> m_aggregate_queue;

      const JsonValue& extract(
        const char* name, boost::optional<JsonValue>& storage);
  };

  template<typename S>
  struct inverse<JsonReceiver<S>> {
    using type = JsonSender<S>;
  };

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
      value = boost::get<bool>(json_value);
    } catch(const boost::bad_get&) {
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
    if(auto s = boost::get<std::string>(&json_value)) {
      if(s->size() != 1) {
        boost::throw_with_location(
          SerializationException("Length out of range."));
      }
      value = s->front();
    } else if(boost::get<double>(&json_value)) {
      value = '\0';
    } else {
      boost::throw_with_location(SerializationException("JSON type mismatch."));
    }
  }

  template<IsConstBuffer S>
  template<typename T> requires std::is_integral_v<T>
  void JsonReceiver<S>::receive(const char* name, T& value) {
    auto raw_value = double();
    receive(name, raw_value);
    value = static_cast<T>(raw_value);
  }

  template<IsConstBuffer S>
  template<typename T> requires std::is_floating_point_v<T>
  void JsonReceiver<S>::receive(const char* name, T& value) {
    auto storage = boost::optional<JsonValue>();
    auto& json_value = extract(name, storage);
    if(auto s = boost::get<double>(&json_value)) {
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
    if(auto s = boost::get<std::string>(&json_value)) {
      value = std::move(*s);
    } else {
      boost::throw_with_location(SerializationException("JSON type mismatch."));
    }
  }

  template<IsConstBuffer S>
  template<std::size_t N>
  void JsonReceiver<S>::receive(const char* name, FixedString<N>& value) {
    auto storage = boost::optional<JsonValue>();
    auto& json_value = extract(name, storage);
    if(auto s = boost::get<std::string>(&json_value)) {
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
    if(auto s = boost::get<JsonObject>(&json_value)) {
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
    if(auto s = boost::get<std::vector<JsonValue>>(&json_value)) {
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
        boost::get<JsonObject>(&m_aggregate_queue.back()))  {
      if(name) {
        return aggregate->at(name);
      } else {
        boost::throw_with_location(
          SerializationException("Invalid JSON format."));
      }
    } else if(auto aggregate =
        boost::get<Sequence>(&m_aggregate_queue.back())) {
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
