#ifndef BEAM_JSONPARSER_HPP
#define BEAM_JSONPARSER_HPP
#include <sstream>
#include <boost/throw_exception.hpp>
#include <boost/variant/get.hpp>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Json/Json.hpp"
#include "Beam/Json/JsonObject.hpp"
#include "Beam/Json/JsonValue.hpp"
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/ForListParser.hpp"
#include "Beam/Parsers/ListParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/RuleParser.hpp"
#include "Beam/Parsers/Types.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {

  /** Parses JSON objects. */
  inline const auto& JsonParser() {
    static const auto value = [] {
      auto objectParser = Parsers::RuleParser<JsonObject>();
      auto arrayParser = Parsers::RuleParser<std::vector<JsonValue>>();
      auto nullParser = Parsers::Symbol("null", JsonNull());
      auto keyParser = Parsers::string_p;
      auto valueParser = Parsers::Convert(Parsers::string_p | nullParser |
        Parsers::bool_p | Parsers::double_p | objectParser | arrayParser,
        [] (const Details::JsonVariant& value) {
          return JsonValue(value);
        });
      auto keyValueParser = Parsers::tokenize >> keyParser >> ':' >>
        valueParser;
      objectParser.SetRule(Parsers::tokenize >> '{' >>
        Parsers::ForList(JsonObject(), keyValueParser, ',',
          [] (JsonObject& object,
              const std::tuple<std::string, JsonValue>& value) {
            object.Set(std::get<0>(value), std::get<1>(value));
          }) >> '}');
      arrayParser.SetRule(Parsers::tokenize >> '[' >>
        Parsers::List(valueParser, ',') >> ']');
      return valueParser;
    }();
    return value;
  }
}

namespace Beam::Serialization {
  template<>
  struct IsStructure<JsonObject> : std::false_type {};

  template<>
  struct Send<JsonObject> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const JsonObject& value) const {
      auto destination = std::stringstream();
      value.Save(destination);
      shuttle.Send(name, destination.str());
    }
  };

  template<>
  struct Receive<JsonObject> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        JsonObject& value) const {
      auto data = std::string();
      shuttle.Shuttle(name, data);
      auto stream = Parsers::ReaderParserStream<
        IO::BufferReader<IO::SharedBuffer>>(
        IO::BufferFromString<IO::SharedBuffer>(data));
      auto jsonValue = JsonValue();
      if(!JsonParser().Read(stream, jsonValue) ||
          boost::get<JsonObject>(&jsonValue) == nullptr) {
        BOOST_THROW_EXCEPTION(SerializationException("Invalid JSON object."));
      }
      value = boost::get<JsonObject>(jsonValue);
    }
  };
}

#endif
