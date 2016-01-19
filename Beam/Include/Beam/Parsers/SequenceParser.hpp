#ifndef BEAM_SEQUENCEPARSER_HPP
#define BEAM_SEQUENCEPARSER_HPP
#include <vector>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SkipSpaceParser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {
namespace Parsers {

  /*! \class SequenceParser
      \brief Matches a sequence of Parsers seperated by a delimiter.
      \tparam ParserType The Parser used for each value in the list.
   */
  template<typename ParserType>
  class SequenceParser : public ParserOperators {
    public:

      //! The Parser used for each value in the list.
      typedef ParserType Parser;
      typedef typename std::conditional<std::is_same<typename Parser::Result,
        NullType>::value, NullType, std::vector<typename Parser::Result>>::type
        Result;

      //! Constructs a SequenceParser.
      /*!
        \param parsers The sequence of Parsers to match.
        \param delimiter The delimiter used to separate list items.
      */
      SequenceParser(const std::vector<Parser>& parsers, char delimiter);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);

    private:
      std::vector<Parser> m_parsers;
      char m_delimiter;
  };

  //! Builds a SequenceParser based on the list of Parsers passed to it.
  /*!
    \param parsers The sequence of Parsers to match.
    \param delimiter The delimiter used to separate list items.
  */
  template<typename ParserType>
  SequenceParser<ParserType> Sequence(const std::vector<ParserType>& parsers,
      char delimiter) {
    return SequenceParser<ParserType>(parsers, delimiter);
  }

  template<typename ParserType>
  SequenceParser<ParserType>::SequenceParser(const std::vector<Parser>& parsers,
      char delimiter)
      : m_parsers(parsers),
        m_delimiter(delimiter) {}

  template<typename ParserType>
  template<typename ParserStreamType>
  bool SequenceParser<ParserType>::Read(ParserStreamType& source,
      Result& value) {
    SubParserStream<ParserStreamType> context(source);
    value.clear();
    for(auto i = m_parsers.begin(); i != m_parsers.end(); ++i) {
      SkipSpaceParser().Read(context);
      if(i != m_parsers.begin()) {
        if(!context.Read()) {
          return false;
        }
        if(context.GetChar() != m_delimiter) {
          return false;
        }
        SkipSpaceParser().Read(context);
      }
      typename Parser::Result listValue;
      if(!i->Read(context, listValue)) {
        return false;
      }
      value.push_back(std::move(listValue));
    }
    context.Accept();
    return true;
  }

  template<typename ParserType>
  template<typename ParserStreamType>
  bool SequenceParser<ParserType>::Read(ParserStreamType& source) {
    SubParserStream<ParserStreamType> context(source);
    for(auto i = m_parsers.begin(); i != m_parsers.end(); ++i) {
      SkipSpaceParser().Read(context);
      if(i != m_parsers.begin()) {
        if(!context.Read()) {
          return false;
        }
        if(context.GetChar() != m_delimiter) {
          return false;
        }
        SkipSpaceParser().Read(context);
      }
      if(!i->Read(context)) {
        return false;
      }
    }
    context.Accept();
    return true;
  }
}
}

#endif
