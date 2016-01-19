#ifndef BEAM_CUSTOMPARSER_HPP
#define BEAM_CUSTOMPARSER_HPP
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/RuleParser.hpp"

namespace Beam {
namespace Parsers {

  /*! \class CustomParser
      \brief Used as a base class to a custom rule based parser.
      \tparam ResultType The data type storing the parsed value.
   */
  template<typename ResultType>
  class CustomParser : public ParserOperators {
    public:
      typedef ResultType Result;

      //! Constructs a CustomParser.
      CustomParser();

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);

    protected:
      template<typename ParserType>
      void SetRule(const ParserType& parser);

    private:
      RuleParser<Result> m_parser;
  };

  template<typename ResultType>
  CustomParser<ResultType>::CustomParser() {}

  template<typename ResultType>
  template<typename ParserStreamType>
  bool CustomParser<ResultType>::Read(ParserStreamType& source, Result& value) {
    return m_parser.Read(source, value);
  }

  template<typename ResultType>
  template<typename ParserStreamType>
  bool CustomParser<ResultType>::Read(ParserStreamType& source) {
    return m_parser.Read(source);
  }

  template<typename ResultType>
  template<typename ParserType>
  void CustomParser<ResultType>::SetRule(const ParserType& parser) {
    m_parser = parser;
  }
}
}

#endif
