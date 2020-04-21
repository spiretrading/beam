#ifndef BEAM_PARSERS_PARSER_HPP
#define BEAM_PARSERS_PARSER_HPP
#include <string>
#include <type_traits>
#include <vector>
#include <boost/throw_exception.hpp>
#include "Beam/Parsers/ListParser.hpp"
#include "Beam/Parsers/ParserException.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/Traits.hpp"

namespace Beam::Parsers {

  /** Specifies the default parser to use for a given type. */
  template<typename T, typename = void>
  const auto default_parser = std::enable_if_t<!std::is_same_v<T, T>>();

  template<typename T>
  const auto default_parser<std::vector<T>> = [] {
    return '[' >> List(default_parser<T>, ',') >> ']';
  }();

  //! Parses a value from a string.
  /*!
    \param source The string to parse.
    \return The parsed value.
  */
  template<typename Parser>
  auto ParseFrom(const Parser& parser, const std::string& source) {
    auto value = typename to_parser_t<Parser>::Result();
    auto stream = ParserStreamFromString(source);
    if(!to_parser_t<Parser>(parser).Read(stream, value)) {
      BOOST_THROW_EXCEPTION(ParserException("Invalid value."));
    }
    return value;
  }

  //! Parses a value from a buffer.
  /*!
    \param source The string to parse.
    \return The parsed value.
  */
  template<typename Parser, typename Buffer>
  auto ParseFrom(const Parser& parser, const Buffer& source) {
    auto value = typename to_parser_t<Parser>::Result();
    auto stream = ReaderParserStream(source);
    if(!to_parser_t<Parser>(parser).Read(stream, value)) {
      BOOST_THROW_EXCEPTION(ParserException("Invalid value."));
    }
    return value;
  }

  template<typename T>
  auto Parse(const std::string& source) {
    return ParseFrom(default_parser<T>, source);
  }

  template<typename T, typename Buffer>
  auto Parse(const Buffer& source) {
    return ParseFrom(default_parser<T>, source);
  }
}

#endif
