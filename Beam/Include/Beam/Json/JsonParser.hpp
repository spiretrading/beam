#ifndef BEAM_JSON_PARSER_HPP
#define BEAM_JSON_PARSER_HPP
#include <sstream>
#include <boost/throw_exception.hpp>
#include <boost/variant/get.hpp>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Json/JsonObject.hpp"
#include "Beam/Json/JsonValue.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {

  /** Parses JSON objects. */
  inline const auto json_p = [] {
    auto object_parser = DeferredParser<JsonObject>();
    auto array = DeferredParser<std::vector<JsonValue>>();
    auto null_parser = symbol("null", JsonNull());
    auto key_parser = string_p;
    auto value_parser = convert(string_p | null_parser | bool_p | double_p |
      object_parser | array, [] (const auto& value) {
        return JsonValue(value);
      });
    auto key_value_pair_parser = tokenize(key_parser, ':', value_parser);
    object_parser.set(tokenize('{',
        for_list(key_value_pair_parser, JsonObject(), ',',
        [] (auto& object, const auto& value) {
          object.set(std::get<0>(value), std::get<1>(value));
        }), '}'));
    array.set(tokenize('[', list(value_parser, ','), ']'));
    return value_parser;
  }();

  template<>
  inline const auto default_parser<JsonValue> = json_p;

  template<>
  constexpr auto is_structure<JsonObject> = false;

  template<>
  struct Send<JsonObject> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const JsonObject& value) const {
      auto destination = std::stringstream();
      value.save(destination);
      sender.send(name, destination.str());
    }
  };

  template<>
  struct Receive<JsonObject> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name, JsonObject& value) const {
      auto data = std::string();
      receiver.receive(name, data);
      auto stream = ReaderParserStream(BufferReader(from<SharedBuffer>(data)));
      auto json_value = JsonValue();
      if(!json_p.read(stream, json_value) ||
          !boost::get<JsonObject>(&json_value)) {
        boost::throw_with_location(
          SerializationException("Invalid JSON object."));
      }
      value = boost::get<JsonObject>(json_value);
    }
  };
}

#endif
