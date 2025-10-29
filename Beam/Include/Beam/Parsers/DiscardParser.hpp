#ifndef BEAM_DISCARD_PARSER_HPP
#define BEAM_DISCARD_PARSER_HPP
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/ParserTraits.hpp"
#include "Beam/Parsers/SpaceParser.hpp"

namespace Beam {

  /**
   * A Parser that discards any parsed value.
   * @tparam P The parser to match and then discard.
   */
  template<IsParser P>
  class DiscardParser {
    public:

      /** The parser to match and then discard. */
      using SubParser = P;
      using Result = void;

      /**
       * Constructs a DiscardParser.
       * @param sub_parser The Parser to match and discard.
       */
      DiscardParser(SubParser sub_parser);

      template<IsParserStream S>
      bool read(S& source) const;

    private:
      SubParser m_sub_parser;
  };

  template<typename P>
  DiscardParser(P) -> DiscardParser<to_parser_t<P>>;

  /** A parser that discards any whitespace. */
  inline const auto space_p = DiscardParser(SpaceParser());

  /**
   * Returns a DiscardParser.
   * @param sub_parser The Parser to discard.
   */
  template<typename P>
  auto discard(P sub_parser) {
    return DiscardParser(std::move(sub_parser));
  }

  template<IsParser P>
  DiscardParser<P>::DiscardParser(SubParser sub_parser)
    : m_sub_parser(std::move(sub_parser)) {}

  template<IsParser P>
  template<IsParserStream S>
  bool DiscardParser<P>::read(S& source) const {
    return m_sub_parser.read(source);
  }
}

#endif
