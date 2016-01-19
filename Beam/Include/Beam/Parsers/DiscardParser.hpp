#ifndef BEAM_DISCARDPARSER_HPP
#define BEAM_DISCARDPARSER_HPP
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {
namespace Parsers {

  /*! \class DiscardParser
      \brief A NullType Parser that discards any parsed value.
      \tparam SubParserType The parser to match and then discard.
   */
  template<typename SubParserType>
  class DiscardParser : public ParserOperators {
    public:
      typedef SubParserType SubParser;
      typedef NullType Result;

      DiscardParser(const SubParser& subParser);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);

    private:
      SubParser m_subParser;
  };

  //! Builds a DiscardParser.
  /*!
    \param subParser The SubParser to discard.
  */
  template<typename SubParser>
  DiscardParser<typename GetParserType<SubParser>::type> Discard(
      const SubParser& subParser) {
    return DiscardParser<typename GetParserType<SubParser>::type>(subParser);
  }

  template<typename SubParserType>
  DiscardParser<SubParserType>::DiscardParser(const SubParser& subParser)
      : m_subParser(subParser) {}

  template<typename SubParserType>
  template<typename ParserStreamType>
  bool DiscardParser<SubParserType>::Read(ParserStreamType& source) {
    return m_subParser.Read(source);
  }
}
}

#endif
