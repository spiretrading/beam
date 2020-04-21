#ifndef BEAM_TOKENPARSER_HPP
#define BEAM_TOKENPARSER_HPP
#include <type_traits>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SkipSpaceParser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Traits.hpp"

namespace Beam::Parsers {

  /*! \class TokenParser
      \brief Parses from a sub Parser that may have leading spaces.
      \tparam P The parser to match with optional leading spaces.
   */
  template<typename P>
  class TokenParser {
    public:

      //! The parser to match with optional leading spaces.
      using SubParser = P;
      using Result = parser_result_t<SubParser>;

      TokenParser(SubParser subParser)
        : m_subParser(std::move(subParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        auto context = SubParserStream<Stream>(source);
        SkipSpaceParser().Read(context);
        if(m_subParser.Read(context, value)) {
          context.Accept();
          return true;
        }
        return false;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        auto context = SubParserStream<Stream>(source);
        SkipSpaceParser().Read(context);
        if(m_subParser.Read(context)) {
          context.Accept();
          return true;
        }
        return false;
      }

    private:
      SubParser m_subParser;
  };

  template<typename P>
  TokenParser(P) -> TokenParser<to_parser_t<P>>;

  //! Builds a TokenParser.
  /*!
    \param subParser The Parser to match.
  */
  template<typename SubParser>
  auto Token(SubParser subParser) {
    return TokenParser(std::move(subParser));
  }

  /*! \class ChainTokenParser
      \brief Chains multiple parsers together to form a chained TokenParser.
      \tparam P The parser to match with optional leading spaces.
   */
  template<typename P>
  class ChainTokenParser {
    public:

      //! The parser to match with optional leading spaces.
      using SubParser = P;
      using Result = parser_result_t<SubParser>;

      //! Constructs a ChainTokenParser.
      /*!
        \param subParser The sub Parser to match.
      */
      ChainTokenParser(SubParser subParser);

      template<typename Stream>
      bool Read(Stream& source) const;

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename RightHandParser>
      auto operator >>(RightHandParser parser) const {
        using SubParser = decltype(m_subParser >> Token(std::move(parser)));
        return ChainTokenParser<std::decay_t<SubParser>>(
          m_subParser >> Token(std::move(parser)));
      }

    private:
      SubParser m_subParser;
  };

  template<typename P>
  ChainTokenParser(P) -> ChainTokenParser<to_parser_t<P>>;

  template<typename P>
  ChainTokenParser<P>::ChainTokenParser(SubParser subParser)
    : m_subParser(std::move(subParser)) {}

  template<typename P>
  template<typename Stream>
  bool ChainTokenParser<P>::Read(Stream& source) const {
    return m_subParser.Read(source);
  }

  template<typename P>
  template<typename Stream>
  bool ChainTokenParser<P>::Read(Stream& source, Result& value) const {
    static_assert(!std::is_same_v<Result, NullType>);
    return m_subParser.Read(source, value);
  }
}

#endif
