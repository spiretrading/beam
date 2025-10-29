#ifndef BEAM_FOR_LIST_PARSER_HPP
#define BEAM_FOR_LIST_PARSER_HPP
#include <cassert>
#include <concepts>
#include "Beam/Parsers/ParserTraits.hpp"
#include "Beam/Parsers/SkipSpaceParser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {

  /**
   * Performs an operation on each value parsed in a list.
   * @tparam P The Parser used for each value in the list.
   * @tparam R The type of value to iteratively update.
   * @tparam M The type of function used to perform updates.
   */
  template<IsParser P, typename R, typename M>
  class ForListParser {
    public:

      /** The Parser used for each value in the list. */
      using Parser = P;

      /** The type of function used to perform updates. */
      using Modifier = M;
  };

  template<IsParser P, typename R, typename M> requires(
    !std::same_as<parser_result_t<P>, void> &&
      std::invocable<M, R&, parser_result_t<P>&>)
  class ForListParser<P, R, M> {
    public:
      using Parser = P;
      using Modifier = M;
      using Result = R;

      /**
       * Constructs a ForListParser.
       * @param parser The Parser used to match each value in the list.
       * @param initial The Parser's initial value.
       * @param delimiter The delimiter used in the list.
       * @param modifier The function used to perform updates.
       */
      ForListParser(
        Parser parser, Result initial, char delimiter, Modifier modifier)
        : m_parser(std::move(parser)),
          m_initial(std::move(initial)),
          m_delimiter(delimiter),
          m_modifier(std::move(modifier)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        value = m_initial;
        auto list_value = parser_result_t<Parser>();
        {
          auto context = SubParserStream<S>(source);
          if(!m_parser.read(context, list_value)) {
            return true;
          }
          m_modifier(value, list_value);
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
          if(!m_parser.read(context, list_value)) {
            return true;
          }
          m_modifier(value, list_value);
          context.accept();
        }
        assert(false);
        return false;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        auto value = Result();
        return read(source, value);
      }

    private:
      Parser m_parser;
      Result m_initial;
      char m_delimiter;
      Modifier m_modifier;
  };

  template<IsParserOf<void> P, typename R, typename M> requires
    std::invocable<M, R&>
  class ForListParser<P, R, M> {
    public:
      using Parser = P;
      using Modifier = M;
      using Result = R;

      ForListParser(
        Parser parser, Result initial, char delimiter, Modifier modifier)
        : m_parser(std::move(parser)),
          m_initial(std::move(initial)),
          m_delimiter(delimiter),
          m_modifier(std::move(modifier)) {}

      template<IsParserStream S>
      bool read(S& source) const {
        auto value = m_initial;
        {
          auto context = SubParserStream<S>(source);
          if(!m_parser.read(context)) {
            return true;
          }
          m_modifier(value);
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
          m_modifier(value);
          context.accept();
        }
        assert(false);
        return false;
      }

    private:
      Parser m_parser;
      Result m_initial;
      char m_delimiter;
      Modifier m_modifier;
  };

  template<typename P, typename R, typename M>
  ForListParser(P, R, char, M) -> ForListParser<
    to_parser_t<P>, std::remove_cvref_t<R>, std::remove_cvref_t<M>>;

  /**
   * Returns a ForListParser.
   * @param parser The Parser used to match each value in the list.
   * @param initial The Parser's initial value.
   * @param delimiter The delimiter used in the list.
   * @param modifier The function used to perform updates.
   */
  template<IsParser P, typename R, typename M>
  auto for_list(P parser, R initial, char delimiter, M modifier) requires(
    !std::same_as<parser_result_t<P>, void> &&
      std::invocable<M, R&, parser_result_t<P>&> ||
    std::same_as<parser_result_t<P>, void> && std::invocable<M, R&>) {
    return ForListParser(
      std::move(parser), std::move(initial), delimiter, std::move(modifier));
  }
}

#endif
