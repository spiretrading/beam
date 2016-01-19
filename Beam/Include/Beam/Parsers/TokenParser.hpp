#ifndef BEAM_TOKENPARSER_HPP
#define BEAM_TOKENPARSER_HPP
#include <type_traits>
#include <boost/utility/declval.hpp>
#include "Beam/Parsers/SkipSpaceParser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam {
namespace Parsers {
namespace Details {
  template<typename LeftHandParser, typename RightHandParser>
  struct TokenConcatenationType {
    using type = decltype(boost::declval<LeftHandParser>() >>
      boost::declval<TokenParser<
      typename GetParserType<RightHandParser>::type>>());
  };
}

  /*! \class TokenParser
      \brief Parses from a sub Parser that may have leading spaces.
      \tparam SubParserType The parser to match with optional leading spaces.
   */
  template<typename SubParserType>
  class TokenParser : public ParserOperators {
    public:

      //! The parser to match with optional leading spaces.
      typedef SubParserType SubParser;
      typedef typename SubParser::Result Result;

      TokenParser(const SubParser& subParser)
          : m_subParser(subParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        SubParserStream<ParserStreamType> context(source);
        SkipSpaceParser().Read(context);
        if(m_subParser.Read(context, value)) {
          context.Accept();
          return true;
        }
        return false;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        SubParserStream<ParserStreamType> context(source);
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

  //! Builds a TokenParser.
  /*!
    \param subParser The Parser to match.
  */
  template<typename SubParser>
  TokenParser<typename GetParserType<SubParser>::type> Token(
      const SubParser& subParser) {
    return TokenParser<typename GetParserType<SubParser>::type>(subParser);
  }

  /*! \class ChainTokenParser
      \brief Chains multiple parsers together to form a chained TokenParser.
      \tparam SubParserType The parser to match with optional leading spaces.
   */
  template<typename SubParserType>
  class ChainTokenParser {
    public:

      //! The parser to match with optional leading spaces.
      typedef SubParserType SubParser;
      typedef typename SubParser::Result Result;

      //! Constructs a ChainTokenParser.
      /*!
        \param subParser The sub Parser to match.
      */
      ChainTokenParser(const SubParser& subParser);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value);

      template<typename RightHandParser>
      auto operator >>(const RightHandParser& parser) const ->
          ChainTokenParser<typename Details::TokenConcatenationType<
          SubParser, RightHandParser>::type> {
        return ChainTokenParser<typename Details::TokenConcatenationType<
          SubParser, RightHandParser>::type>(m_subParser >> Token(parser));
      }

    private:
      SubParser m_subParser;
  };

  template<typename SubParserType>
  ChainTokenParser<SubParserType>::ChainTokenParser(const SubParser& subParser)
      : m_subParser(subParser) {}

  template<typename SubParserType>
  template<typename ParserStreamType>
  bool ChainTokenParser<SubParserType>::Read(ParserStreamType& source) {
    return m_subParser.Read(source);
  }

  template<typename SubParserType>
  template<typename ParserStreamType>
  bool ChainTokenParser<SubParserType>::Read(ParserStreamType& source,
      Result& value) {
    static_assert(!std::is_same<Result, NullType>::value, "");
    return m_subParser.Read(source, value);
  }
}
}

#endif
