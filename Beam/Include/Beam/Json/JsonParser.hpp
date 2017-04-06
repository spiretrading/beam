#ifndef BEAM_JSONPARSER_HPP
#define BEAM_JSONPARSER_HPP
#include <boost/throw_exception.hpp>
#include <boost/variant/get.hpp>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Json/Json.hpp"
#include "Beam/Json/JsonObject.hpp"
#include "Beam/Json/JsonValue.hpp"
#include "Beam/Parsers/BasicParser.hpp"
#include "Beam/Parsers/ForListParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/RuleParser.hpp"
#include "Beam/Parsers/Types.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {

  /*! \class JsonParser
      \brief Implements a Parser for JsonValues.
   */
  class JsonParser : public Parsers::BasicParser<JsonValue> {
    public:

      //! Returns a JsonParser.
      static JsonParser& GetParser();

      //! Constructs a JsonParser.
      JsonParser();

    private:
      static JsonValue BuildJsonValue(const Details::JsonVariant& value);
  };

  inline JsonParser& JsonParser::GetParser() {
    static JsonParser parser;
    return parser;
  }

  inline JsonParser::JsonParser() {
    Parsers::RuleParser<JsonObject> objectParser;
    Parsers::RuleParser<std::vector<JsonValue>> arrayParser;
    auto nullParser = Parsers::Symbol("null", JsonNull());
    auto keyParser = Parsers::string_p;
    auto valueParser = Parsers::Convert(Parsers::string_p | nullParser |
      Parsers::bool_p | Parsers::double_p | objectParser | arrayParser,
      BuildJsonValue);
    auto keyValueParser = Parsers::tokenize >> keyParser >> ':' >> valueParser;
    objectParser = Parsers::tokenize >> '{' >>
      Parsers::ForList(JsonObject(), keyValueParser, ',',
        [] (JsonObject& object,
            const std::tuple<std::string, JsonValue>& value) {
          object.Set(std::get<0>(value), std::get<1>(value));
        }) >> '}';
    arrayParser = Parsers::tokenize >> '[' >>
      Parsers::List(valueParser, ',') >> ']';
    SetParser(valueParser);
  }

  inline JsonValue JsonParser::BuildJsonValue(
      const Details::JsonVariant& value) {
    return JsonValue(value);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsStructure<JsonObject> : std::false_type {};

  template<>
  struct Send<JsonObject> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const JsonObject& value) const {
      std::stringstream destination;
      value.Save(destination);
      shuttle.Send(name, destination.str());
    }
  };

  template<>
  struct Receive<JsonObject> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        JsonObject& value) const {
      std::string data;
      shuttle.Shuttle(name, data);
      Parsers::ReaderParserStream<IO::BufferReader<IO::SharedBuffer>> stream(
        IO::BufferFromString<IO::SharedBuffer>(data));
      JsonValue jsonValue;
      if(!JsonParser::GetParser().Read(stream, jsonValue) ||
          boost::get<JsonObject>(&jsonValue) == nullptr) {
        BOOST_THROW_EXCEPTION(SerializationException("Invalid JSON object."));
      }
      value = boost::get<JsonObject>(jsonValue);
    }
  };
}
}

#endif
