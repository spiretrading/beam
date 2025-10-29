#ifndef BEAM_CONVERSION_PARSER_HPP
#define BEAM_CONVERSION_PARSER_HPP
#include <type_traits>
#include <utility>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/ParserTraits.hpp"

namespace Beam {

  /** 
   * Parses using a sub-parser and then applies a conversion function to the
   * result.
   * @tparam P The type of sub-parser to use.
   * @tparam C The type of conversion function to use.
   */
  template<IsParser P, typename C>
  class ConversionParser {
    public:

      /** The type of sub-parser to use. */
      using SubParser = P;

      /** The type of conversion function to use. */
      using Converter = C;
  };

  template<typename P, typename C>
  ConversionParser(P, C) -> ConversionParser<to_parser_t<P>, C>;

  /**
   * Creates a ConversionParser.
   * @param parser The sub-parser to use.
   * @param f The conversion function to use.
   * @return The constructed ConversionParser.
   */
  template<IsParser P, typename F> requires(
    std::same_as<parser_result_t<P>, void> && std::invocable<F> ||
    !std::same_as<parser_result_t<P>, void> &&
      std::invocable<F, parser_result_t<P>>)
  auto convert(P parser, F f) {
    return ConversionParser(std::move(parser), std::move(f));
  }

  /**
   * Creates a ConversionParser that casts the result of a sub-parser to a
   * different type.
   * @param parser The sub-parser to use.
   * @return The constructed ConversionParser.
   */
  template<typename T, IsParser P> requires(
    !std::same_as<parser_result_t<P>, void>)
  auto cast(P parser) {
    if constexpr(std::is_same_v<parser_result_t<P>, T>) {
      return parser;
    } else {
      return ConversionParser(std::move(parser), [] (auto&& value) {
        return static_cast<T>(std::forward<decltype(value)>(value));
      });
    }
  }

  /**
   * Returns a Parser that matches a symbol and returns a value.
   * @param symbol The symbol to match.
   * @param value The value to return when the <i>symbol</i> is matched.
   */
  template<typename T>
  auto symbol(std::string symbol, T value) {
    return convert(
      SymbolParser(std::move(symbol)), [value = std::move(value)] () -> auto& {
        return value;
      });
  }

  template<IsParserOf<void> P, std::invocable<> C> requires
    std::same_as<std::invoke_result_t<C>, void>
  class ConversionParser<P, C> {
    public:
      using SubParser = P;
      using Converter = C;
      using Result = void;

      /**
       * Constructs a ConversionParser.
       * @param sub_parser The sub-parser to use. 
       * @param converter The conversion function to use.
       */
      ConversionParser(SubParser sub_parser, Converter converter)
        : m_sub_parser(std::move(sub_parser)),
          m_converter(std::move(converter)) {}

      template<IsParserStream S>
      bool read(S& source) const {
        if(!m_sub_parser.read(source)) {
          return false;
        }
        m_converter();
        return true;
      }

    private:
      SubParser m_sub_parser;
      Converter m_converter;
  };

  template<IsParser P, std::invocable<parser_result_t<P>> C> requires(
    !std::same_as<parser_result_t<P>, void> &&
      std::same_as<std::invoke_result_t<C, parser_result_t<P>>, void>)
  class ConversionParser<P, C> {
    public:
      using SubParser = P;
      using Converter = C;
      using Result = void;

      ConversionParser(SubParser sub_parser, Converter converter)
        : m_sub_parser(std::move(sub_parser)),
          m_converter(std::move(converter)) {}

      template<IsParserStream S>
      bool read(S& source) const {
        auto sub_value = parser_result_t<SubParser>();
        if(!m_sub_parser.read(source, sub_value)) {
          return false;
        }
        m_converter(std::move(sub_value));
        return true;
      }

    private:
      SubParser m_sub_parser;
      Converter m_converter;
  };

  template<IsParserOf<void> P, std::invocable<> C> requires(
    !std::same_as<std::invoke_result_t<C>, void>)
  class ConversionParser<P, C> {
    public:
      using SubParser = P;
      using Converter = C;
      using Result = std::remove_cvref_t<std::invoke_result_t<C>>;

      ConversionParser(SubParser sub_parser, Converter converter)
        : m_sub_parser(std::move(sub_parser)),
          m_converter(std::move(converter)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        if(!m_sub_parser.read(source)) {
          return false;
        }
        value = m_converter();
        return true;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        if(!m_sub_parser.read(source)) {
          return false;
        }
        m_converter();
        return true;
      }

    private:
      SubParser m_sub_parser;
      Converter m_converter;
  };

  template<IsParser P, std::invocable<parser_result_t<P>> C> requires(
    !std::same_as<parser_result_t<P>, void> &&
      !std::same_as<std::invoke_result_t<C, parser_result_t<P>>, void>)
  class ConversionParser<P, C> {
    public:
      using SubParser = P;
      using Converter = C;
      using Result = std::remove_cvref_t<
        std::invoke_result_t<Converter, parser_result_t<SubParser>>>;

      ConversionParser(SubParser sub_parser, Converter converter)
        : m_sub_parser(std::move(sub_parser)),
          m_converter(std::move(converter)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        auto sub_value = parser_result_t<SubParser>();
        if(!m_sub_parser.read(source, sub_value)) {
          return false;
        }
        value = m_converter(std::move(sub_value));
        return true;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        auto sub_value = parser_result_t<SubParser>();
        if(!m_sub_parser.read(source, sub_value)) {
          return false;
        }
        m_converter(std::move(sub_value));
        return true;
      }

    private:
      SubParser m_sub_parser;
      Converter m_converter;
  };
}

#endif
