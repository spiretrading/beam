#ifndef BEAM_PLUSPARSER_HPP
#define BEAM_PLUSPARSER_HPP
#include <string>
#include <type_traits>
#include <vector>
#include <boost/optional.hpp>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Traits.hpp"

namespace Beam::Parsers {

  /**
   * Matches a sub Parser one or more times.
   * The result of the parsing is one of:
   * a) NullType if the sub-expression is a NullType Parser.
   * b) An std::string if the sub-expression returns a char or optional<char>.
   * c) An std::vector of the sub-expression's Result.
   * @param <P> The parser to match one or more times.
   */
  template<typename P, typename E>
  class PlusParser {
    public:

      /** The parser that must match one or more times. */
      using SubParser = P;
  };

  template<typename P>
  class PlusParser<P, std::enable_if_t<
      std::is_same_v<parser_result_t<P>, char>>> {
    public:
      using SubParser = P;
      using Result = std::string;

      PlusParser(SubParser subParser)
        : m_subParser(std::move(subParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        value.clear();
        auto nextChar = char();
        if(m_subParser.Read(source, nextChar)) {
          value += nextChar;
        } else {
          return false;
        }
        while(m_subParser.Read(source, nextChar)) {
          value += nextChar;
        }
        return true;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        if(!m_subParser.Read(source)) {
          return false;
        }
        while(m_subParser.Read(source)) {}
        return true;
      }

    private:
      SubParser m_subParser;
  };

  template<typename P>
  class PlusParser<P, std::enable_if_t<
      std::is_same_v<parser_result_t<P>, boost::optional<char>>>> {
    public:
      using SubParser = P;
      using Result = std::string;

      PlusParser(SubParser subParser)
        : m_subParser(std::move(subParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        value.clear();
        auto nextChar = boost::optional<char>();
        if(m_subParser.Read(source, nextChar)) {
          if(nextChar) {
            value += *nextChar;
          }
        } else {
          return false;
        }
        while(m_subParser.Read(source, nextChar)) {
          if(nextChar) {
            value += *nextChar;
          }
        }
        return true;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        if(!m_subParser.Read(source)) {
          return false;
        }
        while(m_subParser.Read(source)) {}
        return true;
      }

    private:
      SubParser m_subParser;
  };

  template<typename P>
  class PlusParser<P, std::enable_if_t<
      !std::is_same_v<parser_result_t<P>, NullType> &&
      !std::is_same_v<parser_result_t<P>, char> &&
      !std::is_same_v<parser_result_t<P>, boost::optional<char>>>> {
    public:
      using SubParser = P;
      using Result = std::vector<parser_result_t<SubParser>>;

      PlusParser(SubParser subParser)
        : m_subParser(std::move(subParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        value.clear();
        auto nextValue = parser_result_t<SubParser>();
        if(m_subParser.Read(source, nextValue)) {
          value.push_back(std::move(nextValue));
        } else {
          return false;
        }
        while(m_subParser.Read(source, nextValue)) {
          value.push_back(std::move(nextValue));
        }
        return true;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        if(!m_subParser.Read(source)) {
          return false;
        }
        while(m_subParser.Read(source)) {}
        return true;
      }

    private:
      SubParser m_subParser;
  };

  template<typename P>
  class PlusParser<P, std::enable_if_t<
      std::is_same_v<parser_result_t<P>, NullType>>> {
    public:
      using SubParser = P;
      using Result = NullType;

      PlusParser(SubParser subParser)
        : m_subParser(std::move(subParser)) {}

      template<typename Stream>
      bool Read(Stream& source) const {
        if(!m_subParser.Read(source)) {
          return false;
        }
        while(m_subParser.Read(source)) {}
        return true;
      }

    private:
      SubParser m_subParser;
  };

  template<typename P>
  PlusParser(P) -> PlusParser<to_parser_t<P>>;
}

#endif
