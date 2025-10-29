#ifndef BEAM_NOT_PARSER_HPP
#define BEAM_NOT_PARSER_HPP
#include <utility>
#include <boost/optional.hpp>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/ParserTraits.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {

  /**
   * Parses the negation of a Parser, resulting in either the next character
   * in the Stream or none if the Stream has reached the end.
   * @tparam P The Parser to negate.
   */
  template<IsParser P>
  class NotParser {
    public:
      using SubParser = P;
      using Result = boost::optional<char>;

      /**
       * Constructs a NotParser.
       * @param sub_parser The Parser to negate.
       */
      NotParser(SubParser sub_parser);

      template<IsParserStream S>
      bool read(S& source, Result& value) const;
      template<IsParserStream S>
      bool read(S& source) const;

    private:
      SubParser m_sub_parser;
  };

  template<typename P>
  NotParser(P) -> NotParser<to_parser_t<P>>;

  /**
   * Constructs a NotParser.
   * @param sub_parser The Parser to negate.
   */
  template<IsParser P>
  auto operator !(P sub_parser) {
    return NotParser(std::move(sub_parser));
  }

  template<IsParser P>
  NotParser<P>::NotParser(SubParser sub_parser)
    : m_sub_parser(std::move(sub_parser)) {}

  template<IsParser P>
  template<IsParserStream S>
  bool NotParser<P>::read(S& source, Result& value) const {
    {
      auto substream = SubParserStream<S>(source);
      if(m_sub_parser.read(substream)) {
        return false;
      }
    }
    if(source.read()) {
      value = source.peek();
    } else {
      value = boost::none;
    }
    return true;
  }

  template<IsParser P>
  template<IsParserStream S>
  bool NotParser<P>::read(S& source) const {
    auto substream = SubParserStream<S>(source);
    if(m_sub_parser.read(substream)) {
      return false;
    }
    source.read();
    return true;
  }
}

#endif
