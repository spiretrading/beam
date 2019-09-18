#ifndef BEAM_RULEPARSER_HPP
#define BEAM_RULEPARSER_HPP
#include <type_traits>
#include "Beam/Parsers/Operators.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/VirtualParser.hpp"
#include "Beam/Parsers/VirtualParserStream.hpp"
#include "Beam/Pointers/UniquePtr.hpp"

namespace Beam {
namespace Parsers {

  /*! \class RuleParser
      \brief Used to represent any generic Parser.
      \tparam ResultType The data type storing the parsed value.
   */
  template<typename ResultType>
  class RuleParser : public ParserOperators {
    public:
      using Result = ResultType;

      //! Constructs a RuleParser.
      RuleParser();

      //! Copies a RuleParser.
      RuleParser(const RuleParser&) = default;

      //! Provides the definition of this parser.
      /*!
        \param parser Specifies the definition of this parser.
        \return <code>*this</code>
      */
      RuleParser& operator =(const RuleParser& parser);

      //! Provides the definition of this parser.
      /*!
        \param parser Specifies the definition of this parser.
        \return <code>*this</code>
      */
      template<typename Parser>
      typename std::enable_if<std::is_same<
        typename Parser::Result, Result>::value, RuleParser&>::type operator =(
        const Parser& parser);

      //! Provides the definition of this parser.
      /*!
        \param parser Specifies the definition of this parser.
        \return <code>*this</code>
      */
      template<typename Parser>
      typename std::enable_if<!std::is_same<
        typename Parser::Result, Result>::value, RuleParser&>::type operator =(
        const Parser& parser);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);

    protected:

      //! Constructs a RuleParser.
      template<typename Parser, typename = std::enable_if_t<
        !std::is_base_of_v<RuleParser, std::decay_t<Parser>>>>
      RuleParser(Parser&& parser);

    private:
      std::shared_ptr<std::unique_ptr<VirtualParser<Result>>> m_source;
  };

  template<typename ResultType>
  RuleParser<ResultType>::RuleParser()
      : m_source(std::make_shared<std::unique_ptr<VirtualParser<Result>>>()) {}

  template<typename ResultType>
  RuleParser<ResultType>& RuleParser<ResultType>::operator =(
      const RuleParser& parser) {
    if(this == &parser) {
      return *this;
    }
    m_source = parser.m_source;
    return *this;
  }

  template<typename ResultType>
  template<typename Parser>
  typename std::enable_if<std::is_same<typename Parser::Result,
      ResultType>::value, RuleParser<ResultType>&>::type
      RuleParser<ResultType>::operator =(const Parser& parser) {
    *m_source = std::make_unique<WrapperParser<Parser>>(parser);
    return *this;
  }

  template<typename ResultType>
  template<typename Parser>
  typename std::enable_if<!std::is_same<typename Parser::Result,
      ResultType>::value, RuleParser<ResultType>&>::type
      RuleParser<ResultType>::operator =(const Parser& parser) {
    using CastParser = decltype(Cast<Result>(parser));
    *m_source = std::make_unique<WrapperParser<CastParser>>(
      Cast<Result>(parser));
    return *this;
  }

  template<typename ResultType>
  template<typename ParserStreamType>
  bool RuleParser<ResultType>::Read(ParserStreamType& source, Result& value) {
    WrapperParserStream<ParserStreamType> context(source);
    return (*m_source)->Read(context, value);
  }

  template<typename ResultType>
  template<typename ParserStreamType>
  bool RuleParser<ResultType>::Read(ParserStreamType& source) {
    WrapperParserStream<ParserStreamType> context(source);
    return (*m_source)->Read(context);
  }

  template<typename ResultType>
  template<typename Parser, typename>
  RuleParser<ResultType>::RuleParser(Parser&& parser)
      : RuleParser() {
    *m_source = std::make_unique<WrapperParser<Parser>>(
      std::forward<Parser>(parser));
  }
}
}

#endif
