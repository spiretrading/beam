#ifndef BEAM_DISCARDPARSER_HPP
#define BEAM_DISCARDPARSER_HPP
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/Traits.hpp"

namespace Beam::Parsers {

  /*! \class DiscardParser
      \brief A NullType Parser that discards any parsed value.
      \tparam P The parser to match and then discard.
   */
  template<typename P>
  class DiscardParser {
    public:
      using SubParser = P;
      using Result = NullType;

      DiscardParser(SubParser subParser);

      template<typename Stream>
      bool Read(Stream& source) const;

    private:
      SubParser m_subParser;
  };

  template<typename P>
  DiscardParser(P) -> DiscardParser<to_parser_t<P>>;

  //! Builds a DiscardParser.
  /*!
    \param subParser The SubParser to discard.
  */
  template<typename SubParser>
  auto Discard(SubParser subParser) {
    return DiscardParser(std::move(subParser));
  }

  template<typename P>
  DiscardParser<P>::DiscardParser(SubParser subParser)
    : m_subParser(std::move(subParser)) {}

  template<typename P>
  template<typename Stream>
  bool DiscardParser<P>::Read(Stream& source) const {
    return m_subParser.Read(source);
  }
}

#endif
