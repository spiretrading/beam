#ifndef BEAM_STARPARSER_HPP
#define BEAM_STARPARSER_HPP
#include <string>
#include <type_traits>
#include <vector>
#include <boost/variant/variant.hpp>
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam {
namespace Parsers {

  /*! \class StarParser
      \brief Parses a sub-expression zero or more times.
             The result of the parsing is one of:
             a) NullType if the sub-expression is a NullType Parser.
             b) An std::string if the sub-expression returns a char.
             c) An std::vector of the sub-expression's Result.
      \tparam SubParserType The parser to match zero or more times.
   */
  template<typename SubParserType, typename Enabled>
  class StarParser {
    public:

      //! The parser that must match zero or more times.
      typedef SubParserType SubParser;
  };

  //! Builds a StarParser.
  /*!
    \param subParser The SubParser to repeat.
  */
  template<typename SubParser>
  StarParser<typename GetParserType<SubParser>::type> Star(
      const SubParser& subParser) {
    return StarParser<typename GetParserType<SubParser>::type>(subParser);
  }

  template<typename SubParserType>
  class StarParser<SubParserType, typename std::enable_if<
      std::is_same<typename SubParserType::Result, char>::value>::type> :
      public ParserOperators {
    public:
      typedef SubParserType SubParser;
      typedef std::string Result;

      StarParser(const SubParser& subParser)
          : m_subParser(subParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        value.clear();
        char nextChar;
        while(m_subParser.Read(source, nextChar)) {
          value += nextChar;
        }
        return true;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        while(m_subParser.Read(source)) {}
        return true;
      }

    private:
      SubParser m_subParser;
  };

  template<typename SubParserType>
  class StarParser<SubParserType, typename std::enable_if<
      !std::is_same<typename SubParserType::Result, char>::value>::type> :
      public ParserOperators {
    public:
      typedef SubParserType SubParser;
      typedef std::vector<typename SubParser::Result> Result;

      StarParser(const SubParser& subParser)
          : m_subParser(subParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        value.clear();
        typename SubParser::Result nextValue;
        while(m_subParser.Read(source, nextValue)) {
          value.push_back(std::move(nextValue));
        }
        return true;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        while(m_subParser.Read(source)) {}
        return true;
      }

    private:
      SubParser m_subParser;
  };
}
}

#endif
