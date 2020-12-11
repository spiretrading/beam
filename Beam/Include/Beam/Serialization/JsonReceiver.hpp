#ifndef BEAM_JSON_RECEIVER_HPP
#define BEAM_JSON_RECEIVER_HPP
#include <cstdint>
#include <cstring>
#include <deque>
#include <type_traits>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Json/JsonParser.hpp"
#include "Beam/Parsers/RuleParser.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/ReceiverMixin.hpp"
#include "Beam/Serialization/SerializationException.hpp"
#include "Beam/Utilities/FixedString.hpp"

namespace Beam {
namespace Serialization {

  /**
   * Implements a Receiver using the JSON format.
   * @param <S> The type of Buffer to receive the data from.
   */
  template<typename S>
  class JsonReceiver : public ReceiverMixin<JsonReceiver<S>> {
    public:
      static_assert(ImplementsConcept<S, IO::Buffer>::value,
        "S must implement the Buffer Concept.");
      using Source = S;

      /** Constructs a JsonReceiver. */
      JsonReceiver();

      /**
       * Constructs a JsonReceiver.
       * @param registry The TypeRegistry used for receiving polymorphic types.
       */
      JsonReceiver(Ref<const TypeRegistry<JsonSender<Source>>> registry);

      void SetSource(Ref<const Source> source);

      void Shuttle(const char* name, bool& value);

      void Shuttle(const char* name, unsigned char& value);

      void Shuttle(const char* name, signed char& value);

      void Shuttle(const char* name, char& value);

      template<typename T>
      std::enable_if_t<std::is_integral_v<T>> Shuttle(const char* name,
        T& value);

      template<typename T>
      std::enable_if_t<std::is_floating_point_v<T>> Shuttle(const char* name,
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

      using ReceiverMixin<JsonReceiver>::Shuttle;

    private:
      struct Sequence {
        std::vector<JsonValue> m_list;
        std::size_t m_index;
      };
      boost::optional<Parsers::ReaderParserStream<IO::BufferReader<Source>>>
        m_parserStream;
      Parsers::RuleParser<JsonValue> m_parser;
      using AggregateType = boost::variant<JsonObject, Sequence>;
      std::deque<AggregateType> m_aggregateQueue;

      const JsonValue& ExtractValue(const char* name,
        boost::optional<JsonValue>& storage);
  };

  template<typename S>
  JsonReceiver<S>::JsonReceiver()
    : m_parser(JsonParser()) {}

  template<typename S>
  JsonReceiver<S>::JsonReceiver(
    Ref<const TypeRegistry<JsonSender<Source>>> registry)
    : ReceiverMixin<JsonReceiver>(Ref(registry)),
      m_parser(JsonParser()) {}

  template<typename S>
  void JsonReceiver<S>::SetSource(Ref<const Source> source) {
    m_aggregateQueue.clear();
    m_parserStream.emplace(*source);
  }

  template<typename S>
  void JsonReceiver<S>::Shuttle(const char* name, bool& value) {
    auto storage = boost::optional<JsonValue>();
    auto& jsonValue = ExtractValue(name, storage);
    try {
      value = boost::get<bool>(jsonValue);
    } catch(const boost::bad_get&) {
      BOOST_THROW_EXCEPTION(SerializationException("JSON type mismatch."));
    }
  }

  template<typename S>
  void JsonReceiver<S>::Shuttle(const char* name, unsigned char& value) {
    auto numericValue = int();
    Shuttle(name, numericValue);
    if(numericValue < std::numeric_limits<unsigned char>::min() ||
        numericValue > std::numeric_limits<unsigned char>::max()) {
      BOOST_THROW_EXCEPTION(SerializationException("Value out of range."));
    }
    value = static_cast<unsigned char>(numericValue);
  }

  template<typename S>
  void JsonReceiver<S>::Shuttle(const char* name, signed char& value) {
    auto numericValue = int();
    Shuttle(name, numericValue);
    if(numericValue < std::numeric_limits<signed char>::min() ||
        numericValue > std::numeric_limits<signed char>::max()) {
      BOOST_THROW_EXCEPTION(SerializationException("Value out of range."));
    }
    value = static_cast<signed char>(numericValue);
  }

  template<typename S>
  void JsonReceiver<S>::Shuttle(const char* name, char& value) {
    auto storage = boost::optional<JsonValue>();
    auto& jsonValue = ExtractValue(name, storage);
    if(auto s = boost::get<std::string>(&jsonValue)) {
      if(s->size() != 1) {
        BOOST_THROW_EXCEPTION(SerializationException("Length out of range."));
      }
      value = s->front();
    } else if(boost::get<double>(&jsonValue)) {
      value = '\0';
    } else {
      BOOST_THROW_EXCEPTION(SerializationException("JSON type mismatch."));
    }
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<std::is_integral_v<T>> JsonReceiver<S>::Shuttle(
      const char* name, T& value) {
    auto rawValue = double();
    Shuttle(name, rawValue);
    value = static_cast<T>(rawValue);
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<std::is_floating_point_v<T>> JsonReceiver<S>::Shuttle(
      const char* name, T& value) {
    auto storage = boost::optional<JsonValue>();
    auto& jsonValue = ExtractValue(name, storage);
    if(auto s = boost::get<double>(&jsonValue)) {
      value = static_cast<T>(*s);
    } else {
      BOOST_THROW_EXCEPTION(SerializationException("JSON type mismatch."));
    }
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<ImplementsConcept<T, IO::Buffer>::value>
    JsonReceiver<S>::Shuttle(const char* name, T& value) {}

  template<typename S>
  void JsonReceiver<S>::Shuttle(const char* name, std::string& value) {
    auto storage = boost::optional<JsonValue>();
    auto& jsonValue = ExtractValue(name, storage);
    if(auto s = boost::get<std::string>(&jsonValue)) {
      value = std::move(*s);
    } else {
      BOOST_THROW_EXCEPTION(SerializationException("JSON type mismatch."));
    }
  }

  template<typename S>
  template<std::size_t N>
  void JsonReceiver<S>::Shuttle(const char* name, FixedString<N>& value) {
    auto storage = boost::optional<JsonValue>();
    auto& jsonValue = ExtractValue(name, storage);
    if(auto s = boost::get<std::string>(&jsonValue)) {
      if(s->size() > N) {
        BOOST_THROW_EXCEPTION(SerializationException("Length out of range."));
      }
      value = std::move(*s);
    } else {
      BOOST_THROW_EXCEPTION(SerializationException("JSON type mismatch."));
    }
  }

  template<typename S>
  void JsonReceiver<S>::StartStructure(const char* name) {
    auto storage = boost::optional<JsonValue>();
    auto& jsonValue = ExtractValue(name, storage);
    if(auto s = boost::get<JsonObject>(&jsonValue)) {
      if(!s->Get("__version")) {
        const_cast<JsonObject&>(*s).Set("__version", 0.0);
      }
      m_aggregateQueue.push_back(*s);
    } else {
      BOOST_THROW_EXCEPTION(SerializationException("JSON type mismatch."));
    }
  }

  template<typename S>
  void JsonReceiver<S>::EndStructure() {
    m_aggregateQueue.pop_back();
  }

  template<typename S>
  void JsonReceiver<S>::StartSequence(const char* name, int& size) {
    auto storage = boost::optional<JsonValue>();
    auto& jsonValue = ExtractValue(name, storage);
    if(auto s = boost::get<std::vector<JsonValue>>(&jsonValue)) {
      auto sequence = Sequence();
      sequence.m_list = std::move(*s);
      sequence.m_index = 0;
      size = static_cast<int>(sequence.m_list.size());
      m_aggregateQueue.push_back(std::move(sequence));
    } else {
      BOOST_THROW_EXCEPTION(SerializationException("JSON type mismatch."));
    }
  }

  template<typename S>
  void JsonReceiver<S>::StartSequence(const char* name) {
    auto dummy = int();
    StartSequence(name, dummy);
  }

  template<typename S>
  void JsonReceiver<S>::EndSequence() {
    m_aggregateQueue.pop_back();
  }

  template<typename S>
  const JsonValue& JsonReceiver<S>::ExtractValue(const char* name,
      boost::optional<JsonValue>& storage) {
    if(m_aggregateQueue.empty()) {
      storage.emplace();
      if(!m_parser.Read(*m_parserStream, *storage)) {
        BOOST_THROW_EXCEPTION(SerializationException("Invalid JSON format."));
      }
      return *storage;
    } else if(auto aggregate =
        boost::get<JsonObject>(&m_aggregateQueue.back()))  {
      if(name != nullptr) {
        return aggregate->At(name);
      } else {
        BOOST_THROW_EXCEPTION(SerializationException("Invalid JSON format."));
      }
    } else if(auto aggregate = boost::get<Sequence>(&m_aggregateQueue.back())) {
      if(aggregate->m_index >= aggregate->m_list.size()) {
        BOOST_THROW_EXCEPTION(
          SerializationException("JSON sequence out of range."));
      }
      auto& value = aggregate->m_list[aggregate->m_index];
      ++aggregate->m_index;
      return value;
    }
    BOOST_THROW_EXCEPTION(SerializationException("Invalid JSON format."));
  }

  template<typename S>
  struct Inverse<JsonReceiver<S>> {
    using type = JsonSender<S>;
  };
}

  template<typename S>
  struct ImplementsConcept<Serialization::JsonReceiver<S>,
    Serialization::Receiver<S>> : std::true_type {};
}

#endif
