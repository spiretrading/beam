#ifndef BEAM_LISTPARSER_HPP
#define BEAM_LISTPARSER_HPP
#include <vector>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SkipSpaceParser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {
namespace Parsers {

  /*! \class ListParser
      \brief Matches a list of values seperated by a delimiter.
      \tparam ParserType The Parser used for each value in the list.
   */
  template<typename ParserType>
  class ListParser : public ParserOperators {
    public:

      //! The Parser used for each value in the list.
      typedef ParserType Parser;
      typedef typename std::conditional<std::is_same<typename Parser::Result,
        NullType>::value, NullType, std::vector<typename Parser::Result>>::type
        Result;

      //! Constructs a ListParser.
      /*!
        \param parser The Parser to match.
        \param delimiter The delimiter used to separate list items.
      */
      ListParser(const Parser& parser, char delimiter);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);

    private:
      Parser m_parser;
      char m_delimiter;
  };

  //! Builds a ListParser based on its value Parser.
  /*!
    \param parser The Parser used for the values in the list.
    \param delimiter The list's delimiter.
  */
  template<typename ParserType>
  ListParser<typename GetParserType<ParserType>::type> List(
      const ParserType& parser, char delimiter) {
    return ListParser<typename GetParserType<ParserType>::type>(parser,
      delimiter);
  }

  template<typename ParserType>
  ListParser<ParserType>::ListParser(const Parser& parser, char delimiter)
      : m_parser(parser),
        m_delimiter(delimiter) {}

  template<typename ParserType>
  template<typename ParserStreamType>
  bool ListParser<ParserType>::Read(ParserStreamType& source, Result& value) {
    value.clear();
    {
      SubParserStream<ParserStreamType> context(source);
      typename Parser::Result listValue;
      if(!m_parser.Read(context, listValue)) {
        return true;
      }
      value.push_back(std::move(listValue));
      context.Accept();
    }
    while(true) {
      SubParserStream<ParserStreamType> context(source);
      SkipSpaceParser().Read(context);
      if(!context.Read()) {
        return true;
      }
      if(context.GetChar() != m_delimiter) {
        return true;
      }
      SkipSpaceParser().Read(context);
      typename Parser::Result listValue;
      if(!m_parser.Read(context, listValue)) {
        return true;
      }
      value.push_back(std::move(listValue));
      context.Accept();
    }
    assert(false);
    return false;
  }

  template<typename ParserType>
  template<typename ParserStreamType>
  bool ListParser<ParserType>::Read(ParserStreamType& source) {
    {
      SubParserStream<ParserStreamType> context(source);
      if(!m_parser.Read(context)) {
        return true;
      }
      context.Accept();
    }
    while(true) {
      SubParserStream<ParserStreamType> context(source);
      SkipSpaceParser().Read(context);
      if(!context.Read()) {
        return true;
      }
      if(context.GetChar() != m_delimiter) {
        return true;
      }
      SkipSpaceParser().Read(context);
      if(!m_parser.Read(context)) {
        return true;
      }
      context.Accept();
    }
    assert(false);
    return false;
  }
}
}

#endif
