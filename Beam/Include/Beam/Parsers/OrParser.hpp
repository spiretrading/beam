#ifndef BEAM_ORPARSER_HPP
#define BEAM_ORPARSER_HPP
#include <type_traits>
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/ParserTraits.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {
namespace Details {
  template<typename T1, typename T2>
  struct or_result {};

  template<typename T1, typename T2> requires std::same_as<T1, T2>
  struct or_result<T1, T2> {
    using type = T1;
  };

  template<typename... L, typename R>
  struct or_result<boost::variant<L...>, R> {
    using type = boost::variant<L..., R>;
  };

  template<typename T1, typename T2>
  using or_result_t = typename or_result<T1, T2>::type;
}

  /**
   * Parses a match if either of its two sub-parsers match.
   * The result of the parsing is one of:
   *   a) void if both L and R result in a void.
   *   b) A boost::optional of L's result if R results in a void.
   *   c) A boost::optional of R's result if L results in a void.
   *   d) A boost::variant of L's result and R's result.
   * @tparam L The parser that must match to the left.
   * @tparam R The parser that must match to the right.
   */
  template<IsParser L, IsParser R>
  class OrParser {
    public:

      /** The parser that must match to the left. */
      using LeftParser = L;

      /** The parser that must match to the right. */
      using RightParser = R;
  };

  template<typename L, typename R>
  OrParser(L, R) -> OrParser<to_parser_t<L>, to_parser_t<R>>;

  template<IsParserOf<void> L, IsParserOf<void> R>
  class OrParser<L, R> {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = void;

      /**
       * Constructs an OrParser.
       * @param left The parser that must match to the left.
       * @param right The parser that must match to the right.
        */
      OrParser(LeftParser left, RightParser right)
        : m_left(std::move(left)),
          m_right(std::move(right)) {}

      template<IsParserStream S>
      bool read(S& source) const {
        {
          auto context = SubParserStream<S>(source);
          if(m_left.read(context)) {
            context.accept();
            return true;
          }
        }
        return m_right.read(source);
      }

    private:
      LeftParser m_left;
      RightParser m_right;
  };

  template<typename L, typename R> requires
    IsParser<std::remove_cvref_t<L>> && IsParser<to_parser_t<R>> ||
    IsParser<to_parser_t<L>> && IsParser<std::remove_cvref_t<R>>
  auto operator |(L left, R right) {
    return OrParser(std::move(left), std::move(right));
  }

  template<IsParserOf<void> L, IsParser R> requires(
    !std::same_as<parser_result_t<R>, void>)
  class OrParser<L, R> {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = boost::optional<parser_result_t<RightParser>>;

      OrParser(LeftParser left, RightParser right)
        : m_left(std::move(left)),
          m_right(std::move(right)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        {
          auto context = SubParserStream<S>(source);
          if(m_left.read(context)) {
            context.accept();
            value = Result();
            return true;
          }
        }
        auto sub_value = parser_result_t<RightParser>();
        if(m_right.read(source, sub_value)) {
          value = std::move(sub_value);
          return true;
        }
        return false;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        {
          auto context = SubParserStream<S>(source);
          if(m_left.read(context)) {
            context.accept();
            return true;
          }
        }
        return m_right.read(source);
      }

    private:
      LeftParser m_left;
      RightParser m_right;
  };

  template<IsParser L, IsParserOf<void> R> requires(
    !std::same_as<parser_result_t<L>, void>)
  class OrParser<L, R> {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = boost::optional<parser_result_t<LeftParser>>;

      OrParser(LeftParser left, RightParser right)
        : m_left(std::move(left)),
          m_right(std::move(right)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        {
          auto context = SubParserStream<S>(source);
          auto sub_value = parser_result_t<LeftParser>();
          if(m_left.read(context, sub_value)) {
            context.accept();
            value = std::move(sub_value);
            return true;
          }
        }
        if(m_right.read(source)) {
          value = Result();
          return true;
        }
        return false;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        {
          auto context = SubParserStream<S>(source);
          if(m_left.read(context)) {
            context.accept();
            return true;
          }
        }
        return m_right.read(source);
      }

    private:
      LeftParser m_left;
      RightParser m_right;
  };

  template<IsParser L, IsParser R> requires(
    (!is_instance_v<L, OrParser> && !is_instance_v<R, OrParser>) &&
      !std::same_as<parser_result_t<L>, void> &&
        !std::same_as<parser_result_t<R>, void>)
  class OrParser<L, R> {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = std::conditional_t<std::same_as<
        parser_result_t<LeftParser>, parser_result_t<RightParser>>,
        parser_result_t<LeftParser>, boost::variant<
          parser_result_t<LeftParser>, parser_result_t<RightParser>>>;

      OrParser(LeftParser left, RightParser right)
        : m_left(std::move(left)),
          m_right(std::move(right)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        {
          auto context = SubParserStream<S>(source);
          auto sub_value = parser_result_t<LeftParser>();
          if(m_left.read(context, sub_value)) {
            context.accept();
            value = std::move(sub_value);
            return true;
          }
        }
        auto sub_value = parser_result_t<RightParser>();
        if(m_right.read(source, sub_value)) {
          value = std::move(sub_value);
          return true;
        }
        return false;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        {
          auto context = SubParserStream<S>(source);
          if(m_left.read(context)) {
            context.accept();
            return true;
          }
        }
        return m_right.read(source);
      }

    private:
      LeftParser m_left;
      RightParser m_right;
  };

  template<IsParser L, IsParser R> requires(
    (is_instance_v<to_parser_t<L>, OrParser> ||
      is_instance_v<to_parser_t<R>, OrParser>) &&
      !std::same_as<parser_result_t<L>, void> &&
        !std::same_as<parser_result_t<R>, void>)
  class OrParser<L, R> {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result =
        typename Details::or_result_t<parser_result_t<L>, parser_result_t<R>>;

      OrParser(LeftParser left, RightParser right)
        : m_left(std::move(left)),
          m_right(std::move(right)) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        {
          auto context = SubParserStream<S>(source);
          auto sub_value = parser_result_t<LeftParser>();
          if(m_left.read(context, sub_value)) {
            context.accept();
            value = std::move(sub_value);
            return true;
          }
        }
        auto sub_value = parser_result_t<RightParser>();
        if(m_right.read(source, sub_value)) {
          value = std::move(sub_value);
          return true;
        }
        return false;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        {
          auto context = SubParserStream<S>(source);
          if(m_left.read(context)) {
            context.accept();
            return true;
          }
        }
        return m_right.read(source);
      }

    private:
      LeftParser m_left;
      RightParser m_right;
  };
}

#endif
