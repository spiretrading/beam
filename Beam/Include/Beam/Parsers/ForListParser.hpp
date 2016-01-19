#ifndef BEAM_FORLISTPARSER_HPP
#define BEAM_FORLISTPARSER_HPP
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SkipSpaceParser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {
namespace Parsers {

  /*! \class ForListParser
      \brief Performs an operation on each value parsed in a list.
      \tparam ParserType The Parser used for each value in the list.
      \tparam ResultType The type of value to iteratively update.
      \tparam ModifierType The type of function used to perform updates.
   */
  template<typename ParserType, typename ResultType, typename ModifierType,
    typename Enabled>
  class ForListParser {
    public:

      //! The Parser used for each value in the list.
      typedef ParserType Parser;
  };

  //! Builds a ForListParser.
  /*!
    \param initialValue The Parser's initial value.
    \param parser The Parser used to match each value in the list.
    \param delimiter The delimiter used in the list.
    \param modifier The function used to perform updates.
  */
  template<typename Parser, typename Result, typename Modifier>
  ForListParser<Parser, Result, Modifier> ForList(const Result& initialValue,
      const Parser& parser, char delimiter, Modifier&& modifier) {
    return ForListParser<Parser, Result, Modifier>(parser, initialValue,
      delimiter, std::forward<Modifier>(modifier));
  }

  template<typename ParserType, typename ResultType, typename ModifierType>
  class ForListParser<ParserType, ResultType, ModifierType,
      typename std::enable_if<
      !std::is_same<typename ParserType::Result, NullType>::value>::type> :
      public ParserOperators {
    public:
      typedef ParserType Parser;
      typedef ResultType Result;
      typedef ModifierType Modifier;

      template<typename ModifierForward>
      ForListParser(const Parser& parser, const Result& initialValue,
          char delimiter, ModifierForward&& modifier)
          : m_parser(parser),
            m_initialValue(initialValue),
            m_delimiter(delimiter),
            m_modifier(std::forward<ModifierForward>(modifier)) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        value = m_initialValue;
        typename Parser::Result listValue;
        {
          SubParserStream<ParserStreamType> context(source);
          if(!m_parser.Read(context, listValue)) {
            return false;
          }
          m_modifier(value, listValue);
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
          if(!m_parser.Read(context, listValue)) {
            return true;
          }
          m_modifier(value, listValue);
          context.Accept();
        }
        assert(false);
        return false;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        Result value;
        return Read(source, value);
      }

    private:
      Parser m_parser;
      Result m_initialValue;
      char m_delimiter;
      Modifier m_modifier;
  };

  template<typename ParserType, typename ResultType, typename ModifierType>
  class ForListParser<ParserType, ResultType, ModifierType,
      typename std::enable_if<
      std::is_same<typename ParserType::Result, NullType>::value>::type> :
      public ParserOperators {
    public:
      typedef ParserType Parser;
      typedef ResultType Result;
      typedef ModifierType Modifier;

      template<typename ModifierForward>
      ForListParser(const Parser& parser, const Result& initialValue,
          char delimiter, ModifierForward&& modifier)
          : m_parser(parser),
            m_initialValue(initialValue),
            m_delimiter(delimiter),
            m_modifier(std::forward<ModifierForward>(modifier)) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        Result value = m_initialValue;
        {
          SubParserStream<ParserStreamType> context(source);
          if(!m_parser.Read(context)) {
            return false;
          }
          m_modifier(value);
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
          m_modifier(value);
          context.Accept();
        }
        assert(false);
        return false;
      }

    private:
      Parser m_parser;
      Result m_initialValue;
      char m_delimiter;
      Modifier m_modifier;
  };
}
}

#endif
