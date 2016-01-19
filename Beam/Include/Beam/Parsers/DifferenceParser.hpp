#ifndef BEAM_DIFFERENCEPARSER_HPP
#define BEAM_DIFFERENCEPARSER_HPP
#include <type_traits>
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam {
namespace Parsers {

  /*! \class DifferenceParser
      \brief Matches if the left hand parser matches but not the right hand
             parser.
      \tparam LeftParserType The parser that must match to the left.
      \tparam RightParserType The parser that must not match to the right.
   */
  template<typename LeftParserType, typename RightParserType>
  class DifferenceParser : public ParserOperators {
    public:

      //! The parser that must match to the left.
      typedef LeftParserType LeftParser;

      //! The parser that must match to the right.
      typedef RightParserType RightParser;
      typedef typename LeftParser::Result Result;

      DifferenceParser(const LeftParser& leftParser,
          const RightParser& rightParser)
          : m_leftParser(leftParser),
            m_rightParser(rightParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        {
          SubParserStream<ParserStreamType> context(source);
          if(m_rightParser.Read(context)) {
            return false;
          }
        }
        if(m_leftParser.Read(source, value)) {
          return true;
        }
        return false;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        {
          SubParserStream<ParserStreamType> context(source);
          if(m_rightParser.Read(context)) {
            return false;
          }
        }
        if(m_leftParser.Read(source)) {
          return true;
        }
        return false;
      }

    private:
      LeftParser m_leftParser;
      RightParser m_rightParser;
  };
}
}

#endif
