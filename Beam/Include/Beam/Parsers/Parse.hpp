#ifndef BEAM_PARSERS_PARSER_HPP
#define BEAM_PARSERS_PARSER_HPP
#include <string>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Parsers/DefaultParser.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/ParserException.hpp"
#include "Beam/Parsers/ParserTraits.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

namespace Beam {

  /**
   * Parses a value from a string.
   * @param parser The parser to use.
   * @param source The string to parse.
   * @return The parsed value.
   */
  template<typename P> requires IsParser<to_parser_t<P>>
  auto parse(const P& parser, const std::string& source) {
    auto value = parser_result_t<P>();
    auto stream = to_parser_stream(source);
    if(!to_parser_t<P>(parser).read(stream, value)) {
      boost::throw_with_location(ParserException("Invalid value."));
    }
    return value;
  }

  /**
   * Parses a value from a buffer.
   * @param parser The parser to use.
   * @param source The buffer to parse.
   * @return The parsed value.
   */
  template<typename P, IsConstBuffer B> requires IsParser<to_parser_t<P>>
  auto parse(const P& parser, const B& source) {
    auto value = parser_result_t<P>();
    auto stream = to_parser_stream(source);
    if(!to_parser_t<P>(parser).read(stream, value)) {
      boost::throw_with_location(ParserException("Invalid value."));
    }
    return value;
  }

  /**
   * Parses a value from a string using a default parser.
   * @param source The string to parse.
   * @return The parsed value.
   */
  template<typename T>
  auto parse(const std::string& source) {
    return parse(default_parser<T>, source);
  }

  /**
   * Parses a value from a buffer using a default parser.
   * @param source The buffer to parse.
   * @return The parsed value.
   */
  template<typename T, IsConstBuffer B>
  auto parse(const B& source) {
    return parse(default_parser<T>, source);
  }
}

#endif
