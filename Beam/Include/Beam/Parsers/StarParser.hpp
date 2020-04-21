#ifndef BEAM_STARPARSER_HPP
#define BEAM_STARPARSER_HPP
#include <string>
#include <type_traits>
#include <vector>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Traits.hpp"

namespace Beam::Parsers {

  /*! \class StarParser
      \brief Parses a sub-expression zero or more times.
             The result of the parsing is one of:
             a) NullType if the sub-expression is a NullType Parser.
             b) An std::string if the sub-expression returns a char.
             c) An std::vector of the sub-expression's Result.
      \tparam P The parser to match zero or more times.
   */
  template<typename P, typename E>
  class StarParser {
    public:

      //! The parser that must match zero or more times.
      using SubParser = P;
  };

  template<typename P>
  class StarParser<P, std::enable_if_t<
      std::is_same_v<parser_result_t<P>, char>>> {
    public:
      using SubParser = P;
      using Result = std::string;

      StarParser(SubParser subParser)
        : m_subParser(std::move(subParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        value.clear();
        auto nextChar = char();
        while(m_subParser.Read(source, nextChar)) {
          value += nextChar;
        }
        return true;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        while(m_subParser.Read(source)) {}
        return true;
      }

    private:
      SubParser m_subParser;
  };

  template<typename P>
  class StarParser<P, std::enable_if_t<
      !std::is_same_v<parser_result_t<P>, char>>> {
    public:
      using SubParser = P;
      using Result = std::vector<parser_result_t<SubParser>>;

      StarParser(SubParser subParser)
        : m_subParser(std::move(subParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        value.clear();
        auto nextValue = parser_result_t<SubParser>();
        while(m_subParser.Read(source, nextValue)) {
          value.push_back(std::move(nextValue));
        }
        return true;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        while(m_subParser.Read(source)) {}
        return true;
      }

    private:
      SubParser m_subParser;
  };

  template<typename SubParser>
  StarParser(SubParser) -> StarParser<to_parser_t<SubParser>>;

  //! Builds a StarParser.
  /*!
    \param subParser The SubParser to repeat.
  */
  template<typename SubParser>
  auto Star(SubParser subParser) {
    return StarParser(std::move(subParser));
  }
}

#endif
