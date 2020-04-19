#ifndef BEAM_NOT_PARSER_HPP
#define BEAM_NOT_PARSER_HPP
#include <utility>
#include <boost/optional.hpp>
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam::Parsers {

  /**
   * Parses the negation of a Parser, resulting in either the next character
   * in the ParserStream or none if the ParserStream has reached the end.
   * @param <P> The Parser to negate.
   */
  template<typename P>
  class NotParser : public ParserOperators {
    public:
      using SubParser = P;
      using Result = boost::optional<char>;

      /**
       * Constructs a NotParser.
       * @param subParser The Parser to negate.
       */
      NotParser(SubParser subParser);

      template<typename ParserStream>
      bool Read(ParserStream& source, Result& value);

      template<typename ParserStream>
      bool Read(ParserStream& source);

    private:
      SubParser m_subParser;
  };

  template<typename SubParser>
  auto Not(SubParser subParser) {
    return NotParser(subParser);
  }

  template<typename P>
  NotParser<P>::NotParser(SubParser subParser)
    : m_subParser(std::move(subParser)) {}

  template<typename P>
  template<typename ParserStream>
  bool NotParser<P>::Read(ParserStream& source, Result& value) {
    auto substream = SubParserStream(source);
    if(m_subParser.Read(substream)) {
      return false;
    }
    if(source.Read()) {
      value = source.GetChar();
    } else {
      value = boost::none;
    }
    return true;
  }

  template<typename P>
  template<typename ParserStream>
  bool NotParser<P>::Read(ParserStream& source) {
    auto substream = SubParserStream(source);
    if(m_subParser.Read(substream)) {
      return false;
    }
    source.Read();
    return true;
  }
}

#endif
