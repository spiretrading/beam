#ifndef BEAM_PLUSPARSER_HPP
#define BEAM_PLUSPARSER_HPP
#include <string>
#include <type_traits>
#include <vector>
#include <boost/variant/variant.hpp>
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam {
namespace Parsers {

  /*! \class PlusParser
      \brief Matches a sub Parser one or more times.
             The result of the parsing is one of:
             a) NullType if the sub-expression is a NullType Parser.
             b) An std::string if the sub-expression returns a char.
             c) An std::vector of the sub-expression's Result.
      \tparam SubParserType The parser to match zero or more times.
   */
  template<typename SubParserType, typename Enabled>
  class PlusParser {
    public:

      //! The parser that must match one or more times.
      typedef SubParserType SubParser;
  };

  template<typename SubParserType>
  class PlusParser<SubParserType, typename std::enable_if<
      std::is_same<typename SubParserType::Result, char>::value>::type> :
      public ParserOperators {
    public:
      typedef SubParserType SubParser;
      typedef std::string Result;

      PlusParser(const SubParser& subParser)
          : m_subParser(subParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        value.clear();
        char nextChar;
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

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        if(!m_subParser.Read(source)) {
          return false;
        }
        while(m_subParser.Read(source)) {}
        return true;
      }

    private:
      SubParser m_subParser;
  };

  template<typename SubParserType>
  class PlusParser<SubParserType, typename std::enable_if<
      !std::is_same<typename SubParserType::Result, NullType>::value &&
      !std::is_same<typename SubParserType::Result, char>::value>::type> :
      public ParserOperators {
    public:
      typedef SubParserType SubParser;
      typedef std::vector<typename SubParser::Result> Result;

      PlusParser(const SubParser& subParser)
          : m_subParser(subParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        value.clear();
        typename SubParser::Result nextValue;
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

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        if(!m_subParser.Read(source)) {
          return false;
        }
        while(m_subParser.Read(source)) {}
        return true;
      }

    private:
      SubParser m_subParser;
  };

  template<typename SubParserType>
  class PlusParser<SubParserType, typename std::enable_if<
      std::is_same<typename SubParserType::Result, NullType>::value>::type> :
      public ParserOperators {
    public:
      typedef SubParserType SubParser;
      typedef NullType Result;

      PlusParser(const SubParser& subParser)
          : m_subParser(subParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        if(!m_subParser.Read(source)) {
          return false;
        }
        while(m_subParser.Read(source)) {}
        return true;
      }

    private:
      SubParser m_subParser;
  };
}
}

#endif
