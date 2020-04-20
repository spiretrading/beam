#ifndef BEAM_NOT_PARSER_HPP
#define BEAM_NOT_PARSER_HPP
#include <utility>
#include <boost/optional.hpp>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Traits.hpp"

namespace Beam::Parsers {

  /**
   * Parses the negation of a Parser, resulting in either the next character
   * in the Stream or none if the Stream has reached the end.
   * @param <P> The Parser to negate.
   */
  template<typename P>
  class NotParser {
    public:
      using SubParser = P;
      using Result = boost::optional<char>;

      /**
       * Constructs a NotParser.
       * @param subParser The Parser to negate.
       */
      NotParser(SubParser subParser);

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;

    private:
      SubParser m_subParser;
  };

  template<typename P>
  NotParser(P) -> NotParser<to_parser_t<P>>;

  template<typename SubParser>
  auto Not(SubParser subParser) {
    return NotParser(subParser);
  }

  template<typename P>
  NotParser<P>::NotParser(SubParser subParser)
    : m_subParser(std::move(subParser)) {}

  template<typename P>
  template<typename Stream>
  bool NotParser<P>::Read(Stream& source, Result& value) const {
    {
      auto substream = SubParserStream<Stream>(source);
      if(m_subParser.Read(substream)) {
        return false;
      }
    }
    if(source.Read()) {
      value = source.GetChar();
    } else {
      value = boost::none;
    }
    return true;
  }

  template<typename P>
  template<typename Stream>
  bool NotParser<P>::Read(Stream& source) const {
    auto substream = SubParserStream<Stream>(source);
    if(m_subParser.Read(substream)) {
      return false;
    }
    source.Read();
    return true;
  }
}

#endif
