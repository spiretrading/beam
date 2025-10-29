#ifndef BEAM_CONCATENATE_PARSER_HPP
#define BEAM_CONCATENATE_PARSER_HPP
#include <tuple>
#include <utility>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/ParserTraits.hpp"
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {
namespace Details {
  template<typename L, typename R>
  auto tuple_cat(L&& left, R&& right) {
    if constexpr(is_instance_v<std::remove_cvref_t<L>, std::tuple> &&
        is_instance_v<std::remove_cvref_t<R>, std::tuple>) {
      return std::tuple_cat(std::forward<L>(left), std::forward<R>(right));
    } else if constexpr(!is_instance_v<std::remove_cvref_t<L>, std::tuple> &&
        !is_instance_v<std::remove_cvref_t<R>, std::tuple>) {
      return std::tuple(std::forward<L>(left), std::forward<R>(right));
    } else if constexpr(!is_instance_v<std::remove_cvref_t<L>, std::tuple>) {
      return std::tuple_cat(
        std::tuple(std::forward<L>(left)), std::forward<R>(right));
    } else {
      return std::tuple_cat(
        std::forward<L>(left), std::tuple(std::forward<R>(right)));
    }
  }
}

  /**
   * Concatenates two parsers so that they must both match in order.
   * The result of the parsing is one of:
   *   a) void if both L and R are Parser<void>.
   *   b) L if only R is a Parser<void>.
   *   c) R if only L is a Parser<void>.
   *   d) An std::tuple of both L's Result and R's Result.
   * @tparam L The parser that must match to the left.
   * @tparam R The parser that must match to the right.
   */
  template<IsParser L, IsParser R>
  class ConcatenateParser {
    public:

      /** The parser that must match to the left. */
      using LeftParser = L;

      /** The parser that must match to the right. */
      using RightParser = R;
  };

  template<typename L, typename R>
  ConcatenateParser(L, R) -> ConcatenateParser<to_parser_t<L>, to_parser_t<R>>;

  template<IsParserOf<void> L, IsParserOf<void> R>
  class ConcatenateParser<L, R> {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = void;

      /**
       * Constructs a ConcatenateParser.
       * @param left The parser that must match to the left.
       * @param right The parser that must match to the right.
       */
      ConcatenateParser(LeftParser left, RightParser right)
        : m_left(std::move(left)),
          m_right(std::move(right)) {}

      template<IsParserStream S>
      bool read(S& source) const {
        auto context = SubParserStream<S>(source);
        if(m_left.read(context) && m_right.read(context)) {
          context.accept();
          return true;
        }
        return false;
      }

    private:
      LeftParser m_left;
      RightParser m_right;
  };

  template<IsParser L, IsParserOf<void> R> requires(
    !std::is_same_v<parser_result_t<L>, void>)
  class ConcatenateParser<L, R> {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = parser_result_t<LeftParser>;

      ConcatenateParser(LeftParser left, RightParser right)
        : m_left(std::move(left)),
          m_right(std::move(right)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        auto context = SubParserStream<S>(source);
        if(m_left.read(context, value) && m_right.read(context)) {
          context.accept();
          return true;
        }
        return false;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        auto context = SubParserStream<S>(source);
        if(m_left.read(context) && m_right.read(context)) {
          context.accept();
          return true;
        }
        return false;
      }

    private:
      LeftParser m_left;
      RightParser m_right;
  };

  template<IsParserOf<void> L, IsParser R> requires(
    !std::is_same_v<parser_result_t<R>, void>)
  class ConcatenateParser<L, R> {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = parser_result_t<RightParser>;

      ConcatenateParser(LeftParser left, RightParser right)
        : m_left(std::move(left)),
          m_right(std::move(right)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        auto context = SubParserStream<S>(source);
        if(m_left.read(context) && m_right.read(context, value)) {
          context.accept();
          return true;
        }
        return false;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        auto context = SubParserStream<S>(source);
        if(m_left.read(context) && m_right.read(context)) {
          context.accept();
          return true;
        }
        return false;
      }

    private:
      LeftParser m_left;
      RightParser m_right;
  };

  template<IsParser L, IsParser R> requires(
    !is_instance_v<to_parser_t<L>, ConcatenateParser> &&
      !is_instance_v<to_parser_t<R>, ConcatenateParser> &&
        !std::is_same_v<parser_result_t<L>, void> &&
          !std::is_same_v<parser_result_t<R>, void>)
  class ConcatenateParser<L, R> {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result =
        std::tuple<parser_result_t<LeftParser>, parser_result_t<RightParser>>;

      ConcatenateParser(LeftParser left, RightParser right)
        : m_left(std::move(left)),
          m_right(std::move(right)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        auto context = SubParserStream<S>(source);
        auto left_value = parser_result_t<LeftParser>();
        if(!m_left.read(context, left_value)) {
          return false;
        }
        auto right_value = parser_result_t<RightParser>();
        if(!m_right.read(context, right_value)) {
          return false;
        }
        context.accept();
        value = std::tuple(std::move(left_value), std::move(right_value));
        return true;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        auto context = SubParserStream<S>(source);
        if(m_left.read(context) && m_right.read(context)) {
          context.accept();
          return true;
        }
        return false;
      }

    private:
      LeftParser m_left;
      RightParser m_right;
  };

  template<IsParser L, IsParser R> requires(
    (is_instance_v<to_parser_t<L>, ConcatenateParser> ||
      is_instance_v<to_parser_t<R>, ConcatenateParser>) &&
      !std::is_same_v<parser_result_t<L>, void> &&
        !std::is_same_v<parser_result_t<R>, void>)
  class ConcatenateParser<L, R> {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result =
        decltype(Details::tuple_cat(std::declval<parser_result_t<LeftParser>>(),
          std::declval<parser_result_t<RightParser>>()));

      ConcatenateParser(LeftParser left, RightParser right)
        : m_left(std::move(left)),
          m_right(std::move(right)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        auto context = SubParserStream<S>(source);
        auto left_value = parser_result_t<LeftParser>();
        if(!m_left.read(context, left_value)) {
          return false;
        }
        auto right_value = parser_result_t<RightParser>();
        if(!m_right.read(context, right_value)) {
          return false;
        }
        context.accept();
        value =
          Details::tuple_cat(std::move(left_value), std::move(right_value));
        return true;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        auto context = SubParserStream<S>(source);
        if(m_left.read(context) && m_right.read(context)) {
          context.accept();
          return true;
        }
        return false;
      }

    private:
      LeftParser m_left;
      RightParser m_right;
  };

  /**
   * Concatenates two parsers so that they must both match in order.
   * @param left The parser that must match to the left.
   * @param right The parser that must match to the right.
   * @return A ConcatenateParser that matches <i>left</i> followed by
   *         <i>right</i>.
   */
  template<typename L, typename R> requires
    IsParser<std::remove_cvref_t<L>> && IsParser<to_parser_t<R>> ||
    IsParser<to_parser_t<L>> && IsParser<std::remove_cvref_t<R>>
  auto operator >>(L&& left, R&& right) {
    return ConcatenateParser(std::forward<L>(left), std::forward<R>(right));
  }
}

#endif
