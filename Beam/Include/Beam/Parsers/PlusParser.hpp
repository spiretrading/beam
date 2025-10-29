#ifndef BEAM_PLUS_PARSER_HPP
#define BEAM_PLUS_PARSER_HPP
#include <concepts>
#include <vector>
#include <boost/optional/optional.hpp>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/ParserTraits.hpp"

namespace Beam {

  /**
   * Matches a sub Parser one or more times.
   * The result of the parsing is one of:
   * a) void if the sub-expression is a void Parser.
   * b) An std::string if the sub-expression returns a char or optional<char>.
   * c) An std::vector of the sub-expression's Result.
   * @tparam P The parser to match one or more times.
   */
  template<IsParser P>
  class PlusParser {
    public:

      /** The parser that must match one or more times. */
      using SubParser = P;
  };

  template<typename P>
  PlusParser(P) -> PlusParser<to_parser_t<P>>;

  /**
   * Constructs a PlusParser.
   * @param parser The parser to match one or more times.
   */
  template<IsParser P>
  auto operator +(P parser) {
    return PlusParser(std::move(parser));
  }

  template<IsParserOf<char> P>
  class PlusParser<P> {
    public:
      using SubParser = P;
      using Result = std::string;

      PlusParser(SubParser sub_parser)
        : m_sub_parser(std::move(sub_parser)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        value.clear();
        auto next_char = char();
        if(m_sub_parser.read(source, next_char)) {
          value += next_char;
        } else {
          return false;
        }
        while(m_sub_parser.read(source, next_char)) {
          value += next_char;
        }
        return true;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        if(!m_sub_parser.read(source)) {
          return false;
        }
        while(m_sub_parser.read(source)) {}
        return true;
      }

    private:
      SubParser m_sub_parser;
  };

  template<IsParserOf<boost::optional<char>> P>
  class PlusParser<P> {
    public:
      using SubParser = P;
      using Result = std::string;

      PlusParser(SubParser sub_parser)
        : m_sub_parser(std::move(sub_parser)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        value.clear();
        auto next_char = boost::optional<char>();
        if(m_sub_parser.read(source, next_char)) {
          if(next_char) {
            value += *next_char;
          }
        } else {
          return false;
        }
        while(m_sub_parser.read(source, next_char)) {
          if(next_char) {
            value += *next_char;
          }
        }
        return true;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        if(!m_sub_parser.read(source)) {
          return false;
        }
        while(m_sub_parser.read(source)) {}
        return true;
      }

    private:
      SubParser m_sub_parser;
  };

  template<IsParser P> requires(!std::same_as<parser_result_t<P>, char> &&
    !std::same_as<parser_result_t<P>, boost::optional<char>> &&
      !std::same_as<parser_result_t<P>, void>)
  class PlusParser<P> {
    public:
      using SubParser = P;
      using Result = std::vector<parser_result_t<SubParser>>;

      PlusParser(SubParser sub_parser)
        : m_sub_parser(std::move(sub_parser)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        value.clear();
        auto next_value = parser_result_t<SubParser>();
        if(m_sub_parser.read(source, next_value)) {
          value.push_back(std::move(next_value));
        } else {
          return false;
        }
        while(m_sub_parser.read(source, next_value)) {
          value.push_back(std::move(next_value));
        }
        return true;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        if(!m_sub_parser.read(source)) {
          return false;
        }
        while(m_sub_parser.read(source)) {}
        return true;
      }

    private:
      SubParser m_sub_parser;
  };

  template<IsParserOf<void> P>
  class PlusParser<P> {
    public:
      using SubParser = P;
      using Result = void;

      PlusParser(SubParser sub_parser)
        : m_sub_parser(std::move(sub_parser)) {}

      template<IsParserStream S>
      bool read(S& source) const {
        if(!m_sub_parser.read(source)) {
          return false;
        }
        while(m_sub_parser.read(source)) {}
        return true;
      }

    private:
      SubParser m_sub_parser;
  };
}

#endif
