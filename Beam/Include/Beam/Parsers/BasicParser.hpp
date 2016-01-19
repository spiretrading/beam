#ifndef BEAM_BASICPARSER_HPP
#define BEAM_BASICPARSER_HPP
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/RuleParser.hpp"

namespace Beam {
namespace Parsers {

  /*! \class BasicParser
      \brief Simplifies building a custom Parser.
      \tparam ResultType The data type storing the parsed value.
   */
  template<typename ResultType>
  class BasicParser : public ParserOperators {
    public:
      typedef ResultType Result;

      //! Constructs a BasicParser.
      BasicParser();

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);

    protected:

      //! Provides the definition of this parser.
      /*!
        \param parser Specifies the definition of this parser.
      */
      template<typename Parser>
      void SetParser(const Parser& parser);

    private:
      RuleParser<Result> m_parser;
  };

  template<typename ResultType>
  BasicParser<ResultType>::BasicParser() {}

  template<typename ResultType>
  template<typename ParserStreamType>
  bool BasicParser<ResultType>::Read(ParserStreamType& source, Result& value) {
    return m_parser.Read(source, value);
  }

  template<typename ResultType>
  template<typename ParserStreamType>
  bool BasicParser<ResultType>::Read(ParserStreamType& source) {
    return m_parser.Read(source);
  }

  template<typename ResultType>
  template<typename Parser>
  void BasicParser<ResultType>::SetParser(const Parser& parser) {
    m_parser = parser;
  }
}
}

#endif
