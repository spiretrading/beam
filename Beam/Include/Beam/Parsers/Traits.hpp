#ifndef BEAM_PARSERS_TRAITS_HPP
#define BEAM_PARSERS_TRAITS_HPP
#include <string>
#include <type_traits>
#include <utility>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/SymbolParser.hpp"
#include "Beam/Parsers/TerminalParser.hpp"

namespace Beam::Parsers {

  /** Trait testing whether a type is a parser. */
  template<typename T, typename = void>
  struct is_parser : std::false_type {};

  template<typename T>
  struct is_parser<T, std::enable_if_t<std::is_same_v<
    decltype(std::declval<T>().Read(std::declval<ReaderParserStream<
    IO::BufferReader<IO::SharedBuffer>>&>())), bool>>> : std::true_type {};

  template<typename T>
  constexpr bool is_parser_v = is_parser<T>::value;

  /** Trait used to wrap a type into a parser. */
  template<typename T, typename = void>
  struct to_parser {};

  template<typename T>
  struct to_parser<T, std::enable_if_t<is_parser_v<std::decay_t<T>>>> {
    using type = std::decay_t<T>;
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

  /** Trait testing whether a parser parses a value of type T. */
  template<typename P, typename T>
  struct is_parser_of : std::integral_constant<bool,
    is_parser_v<P> && std::is_same_v<parser_result_t<P>, T>> {};

  template<typename P, typename T>
  constexpr bool is_parser_of_v = is_parser_of<P, T>::value;
}

#endif
