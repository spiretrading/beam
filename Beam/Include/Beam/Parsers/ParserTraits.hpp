#ifndef BEAM_PARSERS_TRAITS_HPP
#define BEAM_PARSERS_TRAITS_HPP
#include <string>
#include <type_traits>
#include <utility>
#include "Beam/Parsers/SymbolParser.hpp"
#include "Beam/Parsers/TerminalParser.hpp"

namespace Beam {

  /** Trait used to wrap a type into a parser. */
  template<typename T>
  struct to_parser {
    using type = std::remove_cvref_t<T>;
  };

  template<>
  struct to_parser<char> {
    using type = TerminalParser;
  };

  template<>
  struct to_parser<const char*> {
    using type = SymbolParser;
  };

  template<>
  struct to_parser<std::string> {
    using type = SymbolParser;
  };

  template<std::size_t N>
  struct to_parser<const char[N]> {
    using type = SymbolParser;
  };

  template<std::size_t N>
  struct to_parser<const char (&)[N]> {
    using type = SymbolParser;
  };

  template<std::size_t N>
  struct to_parser<char[N]> {
    using type = SymbolParser;
  };

  template<typename T>
  using to_parser_t = typename to_parser<T>::type;

  /** Trait used to determine what the result of a parser is. */
  template<typename T>
  struct parser_result {
    using type = typename to_parser_t<T>::Result;
  };

  template<typename T>
  using parser_result_t = typename parser_result<T>::type;
}

#endif
