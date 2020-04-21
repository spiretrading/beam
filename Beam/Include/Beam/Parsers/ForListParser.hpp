#ifndef BEAM_FORLISTPARSER_HPP
#define BEAM_FORLISTPARSER_HPP
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SkipSpaceParser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Traits.hpp"

namespace Beam::Parsers {

  /*! \class ForListParser
      \brief Performs an operation on each value parsed in a list.
      \tparam P The Parser used for each value in the list.
      \tparam R The type of value to iteratively update.
      \tparam M The type of function used to perform updates.
   */
  template<typename P, typename R, typename M, typename E>
  class ForListParser {
    public:

      //! The Parser used for each value in the list.
      using Parser = P;
  };


  template<typename P, typename R, typename M>
  class ForListParser<P, R, M, std::enable_if_t<
      !std::is_same_v<parser_result_t<P>, NullType>>> {
    public:
      using Parser = P;
      using Result = R;
      using Modifier = M;

      ForListParser(Parser parser, Result initialValue, char delimiter,
        Modifier modifier)
        : m_parser(std::move(parser)),
          m_initialValue(std::move(initialValue)),
          m_delimiter(delimiter),
          m_modifier(std::move(modifier)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        value = m_initialValue;
        auto listValue = parser_result_t<Parser>();
        {
          auto context = SubParserStream<Stream>(source);
          if(!m_parser.Read(context, listValue)) {
            return false;
          }
          m_modifier(value, listValue);
          context.Accept();
        }
        while(true) {
          auto context = SubParserStream<Stream>(source);
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

      template<typename Stream>
      bool Read(Stream& source) const {
        auto value = Result();
        return Read(source, value);
      }

    private:
      Parser m_parser;
      Result m_initialValue;
      char m_delimiter;
      Modifier m_modifier;
  };

  template<typename P, typename R, typename M>
  class ForListParser<P, R, M, std::enable_if_t<
      std::is_same_v<parser_result_t<P>, NullType>>> {
    public:
      using Parser = P;
      using Result = R;
      using Modifier = M;

      ForListParser(Parser parser, Result initialValue, char delimiter,
        Modifier modifier)
        : m_parser(std::move(parser)),
          m_initialValue(std::move(initialValue)),
          m_delimiter(delimiter),
          m_modifier(std::move(modifier)) {}

      template<typename Stream>
      bool Read(Stream& source) const {
        auto value = m_initialValue;
        {
          auto context = SubParserStream<Stream>(source);
          if(!m_parser.Read(context)) {
            return false;
          }
          m_modifier(value);
          context.Accept();
        }
        while(true) {
          auto context = SubParserStream<Stream>(source);
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

  template<typename P, typename R, typename M>
  ForListParser(P, R, char, M) -> ForListParser<to_parser_t<P>, std::decay_t<R>,
    std::decay_t<M>>;

  //! Builds a ForListParser.
  /*!
    \param initialValue The Parser's initial value.
    \param parser The Parser used to match each value in the list.
    \param delimiter The delimiter used in the list.
    \param modifier The function used to perform updates.
  */
  template<typename Parser, typename Result, typename Modifier>
  auto ForList(Result initialValue, Parser parser, char delimiter,
      Modifier modifier) {
    return ForListParser(std::move(parser), std::move(initialValue),
      delimiter, std::move(modifier));
  }
}

#endif
