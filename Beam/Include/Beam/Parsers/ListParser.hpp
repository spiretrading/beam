#ifndef BEAM_LIST_PARSER_HPP
#define BEAM_LIST_PARSER_HPP
#include <cassert>
#include <concepts>
#include <type_traits>
#include <vector>
#include "Beam/Parsers/ParserTraits.hpp"
#include "Beam/Parsers/SkipSpaceParser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {

  /**
   * Matches a list of values seperated by a delimiter.
   * @tparam P The Parser used for each value in the list.
   */
  template<IsParser P>
  class ListParser {
    public:

      /** The Parser used for each value in the list. */
      using Parser = P;
  };

  template<typename P>
  ListParser(P, char) -> ListParser<to_parser_t<P>>;

  template<IsParserOf<void> P>
  class ListParser<P> {
    public:
      using Parser = P;
      using Result = void;

      /**
       * Constructs a ListParser.
       * @param parser The Parser to match.
       * @param delimiter The delimiter used to separate list items.
       */
      ListParser(Parser parser, char delimiter)
        : m_parser(std::move(parser)),
          m_delimiter(delimiter) {}

      template<IsParserStream S>
      bool read(S& source) const {
        {
          auto context = SubParserStream<S>(source);
          if(!m_parser.read(context)) {
            return true;
          }
          context.accept();
        }
        while(true) {
          auto context = SubParserStream<S>(source);
          SkipSpaceParser().read(context);
          if(!context.read()) {
            return true;
          }
          if(context.peek() != m_delimiter) {
            return true;
          }
          SkipSpaceParser().read(context);
          if(!m_parser.read(context)) {
            return true;
          }
          context.accept();
        }
        assert(false);
        return false;
      }

    private:
      Parser m_parser;
      char m_delimiter;
  };

  template<IsParser P> requires(!std::same_as<parser_result_t<P>, void>)
  class ListParser<P> {
    public:
      using Parser = P;
      using Result = std::vector<parser_result_t<Parser>>;

      ListParser(Parser parser, char delimiter)
        : m_parser(std::move(parser)),
          m_delimiter(delimiter) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        value.clear();
        {
          auto context = SubParserStream<S>(source);
          auto list_value = parser_result_t<Parser>();
          if(!m_parser.read(context, list_value)) {
            return true;
          }
          value.push_back(std::move(list_value));
          context.accept();
        }
        while(true) {
          auto context = SubParserStream<S>(source);
          SkipSpaceParser().read(context);
          if(!context.read()) {
            return true;
          }
          if(context.peek() != m_delimiter) {
            return true;
          }
          SkipSpaceParser().read(context);
          auto list_value = parser_result_t<Parser>();
          if(!m_parser.read(context, list_value)) {
            return true;
          }
          value.push_back(std::move(list_value));
          context.accept();
        }
        assert(false);
        return false;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        {
          auto context = SubParserStream<S>(source);
          if(!m_parser.read(context)) {
            return true;
          }
          context.accept();
        }
        while(true) {
          auto context = SubParserStream<S>(source);
          SkipSpaceParser().read(context);
          if(!context.read()) {
            return true;
          }
          if(context.peek() != m_delimiter) {
            return true;
          }
          SkipSpaceParser().read(context);
          if(!m_parser.read(context)) {
            return true;
          }
          context.accept();
        }
        assert(false);
        return false;
      }

    private:
      Parser m_parser;
      char m_delimiter;
  };

  /**
   * Returns a ListParser based on its value Parser.
   * @param parser The Parser used for the values in the list.
   * @param delimiter The list's delimiter.
   */
  template<IsParser P>
  auto list(P parser, char delimiter) {
    return ListParser(std::move(parser), delimiter);
  }
}

#endif
