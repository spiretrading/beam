#ifndef BEAM_SEQUENCEPARSER_HPP
#define BEAM_SEQUENCEPARSER_HPP
#include <vector>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SkipSpaceParser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Traits.hpp"

namespace Beam::Parsers {

  /*! \class SequenceParser
      \brief Matches a sequence of Parsers seperated by a delimiter.
      \tparam Parser The Parser used for each value in the list.
   */
  template<typename P>
  class SequenceParser {
    public:

      //! The Parser used for each value in the list.
      using Parser = P;

      using Result = std::conditional_t<std::is_same_v<parser_result_t<Parser>,
        NullType>, NullType, std::vector<parser_result_t<Parser>>>;

      //! Constructs a SequenceParser.
      /*!
        \param parsers The sequence of Parsers to match.
        \param delimiter The delimiter used to separate list items.
      */
      SequenceParser(std::vector<Parser> parsers, char delimiter);

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;

    private:
      std::vector<Parser> m_parsers;
      char m_delimiter;
  };

  template<typename Parser>
  SequenceParser(std::vector<Parser>, char) ->
    SequenceParser<to_parser_t<Parser>>;

  //! Builds a SequenceParser based on the list of Parsers passed to it.
  /*!
    \param parsers The sequence of Parsers to match.
    \param delimiter The delimiter used to separate list items.
  */
  template<typename Parser>
  auto Sequence(std::vector<Parser> parsers, char delimiter) {
    return SequenceParser(std::move(parsers), delimiter);
  }

  template<typename P>
  SequenceParser<P>::SequenceParser(std::vector<Parser> parsers, char delimiter)
    : m_parsers(std::move(parsers)),
      m_delimiter(delimiter) {}

  template<typename P>
  template<typename Stream>
  bool SequenceParser<P>::Read(Stream& source, Result& value) const {
    auto context = SubParserStream<Stream>(source);
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
      auto listValue = parser_result_t<Parser>();
      if(!i->Read(context, listValue)) {
        return false;
      }
      value.push_back(std::move(listValue));
    }
    context.Accept();
    return true;
  }

  template<typename P>
  template<typename Stream>
  bool SequenceParser<P>::Read(Stream& source) const {
    auto context = SubParserStream<Stream>(source);
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

#endif
