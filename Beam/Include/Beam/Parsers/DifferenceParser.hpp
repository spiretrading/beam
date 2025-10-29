#ifndef BEAM_DIFFERENCE_PARSER_HPP
#define BEAM_DIFFERENCE_PARSER_HPP
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/ParserTraits.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {

  /**
   * Matches if the left hand parser matches but not the right hand parser.
   * @tparam L The parser that must match to the left.
   * @tparam R The parser that must not match to the right.
   */
  template<IsParser L, IsParser R>
  class DifferenceParser {
    public:

      /** The parser that must match to the left. */
      using LeftParser = L;

      /** The parser that must not match to the right. */
      using RightParser = R;

      using Result = parser_result_t<LeftParser>;
  };

  template<typename L, typename R>
  DifferenceParser(L, R) -> DifferenceParser<to_parser_t<L>, to_parser_t<R>>;

  template<IsParser L, IsParser R> requires(
    !std::is_same_v<parser_result_t<L>, void>)
  class DifferenceParser<L, R> {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = parser_result_t<LeftParser>;

      /**
       * Constructs a DifferenceParser.
       * @param left The parser that must match to the left.
       * @param right The parser that must not match to the right.
       */
      DifferenceParser(LeftParser left, RightParser right)
        : m_left(std::move(left)),
          m_right(std::move(right)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        {
          auto context = SubParserStream<S>(source);
          if(m_right.read(context)) {
            return false;
          }
        }
        if(m_left.read(source, value)) {
          return true;
        }
        return false;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        {
          auto context = SubParserStream<S>(source);
          if(m_right.read(context)) {
            return false;
          }
        }
        if(m_left.read(source)) {
          return true;
        }
        return false;
      }

    private:
      LeftParser m_left;
      RightParser m_right;
  };

  template<IsParserOf<void> L, IsParser R>
  class DifferenceParser<L, R> {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = parser_result_t<LeftParser>;

      DifferenceParser(LeftParser left, RightParser right)
        : m_left(std::move(left)),
          m_right(std::move(right)) {}

      template<IsParserStream S>
      bool read(S& source) const {
        {
          auto context = SubParserStream<S>(source);
          if(m_right.read(context)) {
            return false;
          }
        }
        if(m_left.read(source)) {
          return true;
        }
        return false;
      }

    private:
      LeftParser m_left;
      RightParser m_right;
  };

  /**
   * Creates a DifferenceParser that matches if the left hand parser matches
   * but not the right hand parser.
   * @param left The parser that must match to the left.
   * @param right The parser that must not match to the right.
   * @return A DifferenceParser that matches if the left hand parser matches
   *         but not the right hand parser.
   */
  template<typename L, typename R> requires
    IsParser<std::remove_cvref_t<L>> && IsParser<to_parser_t<R>> ||
    IsParser<to_parser_t<L>> && IsParser<std::remove_cvref_t<R>>
  auto operator -(L&& left, R&& right) {
    return DifferenceParser(std::forward<L>(left), std::forward<R>(right));
  }
}

#endif
