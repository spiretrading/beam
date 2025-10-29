#ifndef BEAM_STAR_PARSER_HPP
#define BEAM_STAR_PARSER_HPP
#include <string>
#include <type_traits>
#include <vector>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/ParserTraits.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {

  /**
   * Parses a sub-expression zero or more times.
   * The result of the parsing is one of:
   *   a) void if the sub-expression is a void Parser.
   *   b) An std::string if the sub-expression returns a char.
   *   c) An std::vector of the sub-expression's Result.
   * @tparam P The parser to match zero or more times.
   */
  template<IsParser P>
  class StarParser {
    public:

      /** The parser that must match zero or more times. */
      using SubParser = P;
  };

  template<typename P>
  StarParser(P) -> StarParser<to_parser_t<P>>;

  /**
   * Returns a StarParser.
   * @param sub_parser The SubParser to repeat.
   */
  template<IsParser P>
  auto star(P sub_parser) {
    return StarParser(std::move(sub_parser));
  }

  /**
   * Returns a StarParser.
   * @param parser The Parser to repeat.
   */
  template<IsParser P>
  auto operator *(P parser) {
    return StarParser(std::move(parser));
  }

  template<IsParser P> requires(!std::same_as<parser_result_t<P>, char> &&
    !std::same_as<parser_result_t<P>, void>)
  class StarParser<P> {
    public:
      using SubParser = P;
      using Result = std::vector<parser_result_t<SubParser>>;

      /**
       * Constructs a StarParser.
       * @param sub_parser The SubParser to repeat.
       */
      StarParser(SubParser sub_parser)
        : m_sub_parser(std::move(sub_parser)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        value.clear();
        auto next_value = parser_result_t<SubParser>();
        while(m_sub_parser.read(source, next_value)) {
          value.push_back(std::move(next_value));
        }
        return true;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        while(m_sub_parser.read(source)) {}
        return true;
      }

    private:
      SubParser m_sub_parser;
  };

  template<IsParserOf<void> P>
  class StarParser<P> {
    public:
      using SubParser = P;
      using Result = void;

      /**
       * Constructs a StarParser.
       * @param sub_parser The SubParser to repeat.
       */
      StarParser(SubParser sub_parser)
        : m_sub_parser(std::move(sub_parser)) {}

      template<IsParserStream S>
      bool read(S& source) const {
        while(m_sub_parser.read(source)) {}
        return true;
      }

    private:
      SubParser m_sub_parser;
  };

  template<IsParserOf<char> P>
  class StarParser<P> {
    public:
      using SubParser = P;
      using Result = std::string;

      StarParser(SubParser sub_parser)
        : m_sub_parser(std::move(sub_parser)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        value.clear();
        auto next_char = char();
        while(m_sub_parser.read(source, next_char)) {
          value += next_char;
        }
        return true;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        while(m_sub_parser.read(source)) {}
        return true;
      }

    private:
      SubParser m_sub_parser;
  };
}

#endif
