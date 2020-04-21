#ifndef BEAM_CONCATENATEPARSER_HPP
#define BEAM_CONCATENATEPARSER_HPP
#include <tuple>
#include <type_traits>
#include <utility>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Traits.hpp"
#include "Beam/Utilities/NullType.hpp"

namespace Beam::Parsers {
namespace Details {
  template<typename T1, typename T2>
  struct ConcatenateResult {
    using type = std::tuple<T1, T2>;
  };

  template<typename T>
  struct ConcatenateResult<T, NullType> {
    using type = T;
  };

  template<typename T>
  struct ConcatenateResult<NullType, T> {
    using type = T;
  };

  template<typename... T1, typename... T2>
  struct ConcatenateResult<std::tuple<T1...>, std::tuple<T2...>> {
    using type = std::tuple<T1..., T2...>;
  };

  template<typename T1, typename... T2>
  struct ConcatenateResult<T1, std::tuple<T2...>> {
    using type = std::tuple<T1, T2...>;
  };

  template<typename T1, typename... T2>
  struct ConcatenateResult<std::tuple<T2...>, T1> {
    using type = std::tuple<T2..., T1>;
  };

  template<typename... T1, typename... T2>
  std::tuple<T1..., T2...> TupleCat(std::tuple<T1...>&& t1,
      std::tuple<T2...>&& t2) {
    return std::tuple_cat(std::move(t1), std::move(t2));
  }

  template<typename T1, typename... T2>
  std::tuple<T1, T2...> TupleCat(T1&& t1, std::tuple<T2...>&& t2) {
    return std::tuple_cat(std::make_tuple(std::forward<T1>(t1)), std::move(t2));
  }

  template<typename T1, typename... T2>
  std::tuple<T2..., T1> TupleCat(std::tuple<T2...>&& t2, T1&& t1) {
    return std::tuple_cat(std::move(t2), std::make_tuple(std::forward<T1>(t1)));
  }

  template<typename T1, typename T2>
  std::tuple<T1, T2> TupleCat(T1&& t1, T2&& t2) {
    return std::make_tuple(std::forward<T1>(t1), std::forward<T2>(t2));
  }
}

  /*! \class ConcatenateParser
      \brief Concatenates two parsers so that they must both match in order.
             The result of the parsing is one of:
             a) NullType if both L and R are NullType Parsers.
             b) L if only R is a NullType Parser.
             c) R if only L is a NullType Parser.
             d) An std::tuple of both L's Result and R's Result.
      \tparam L The parser that must match to the left.
      \tparam R The parser that must match to the right.
   */
  template<typename L, typename R, typename E>
  class ConcatenateParser {
    public:

      //! The parser that must match to the left.
      using LeftParser = L;

      //! The parser that must match to the right.
      using RightParser = R;
  };

  struct BaseConcatenateParser {};

  template<typename L, typename R>
  class ConcatenateParser<L, R, std::enable_if_t<
      std::is_same_v<parser_result_t<L>, NullType> &&
      std::is_same_v<parser_result_t<R>, NullType>>> :
      public BaseConcatenateParser {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = NullType;

      ConcatenateParser(LeftParser leftParser, RightParser rightParser)
        : m_leftParser(std::move(leftParser)),
          m_rightParser(std::move(rightParser)) {}

      template<typename Stream>
      bool Read(Stream& source) const {
        auto context = SubParserStream<Stream>(source);
        if(m_leftParser.Read(context) && m_rightParser.Read(context)) {
          context.Accept();
          return true;
        }
        return false;
      }

    private:
      LeftParser m_leftParser;
      RightParser m_rightParser;
  };

  template<typename L, typename R>
  class ConcatenateParser<L, R, std::enable_if_t<
      std::is_same_v<parser_result_t<L>, NullType> &&
      !std::is_same_v<parser_result_t<R>, NullType>>> :
      public BaseConcatenateParser {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = parser_result_t<RightParser>;

      ConcatenateParser(LeftParser leftParser, RightParser rightParser)
        : m_leftParser(std::move(leftParser)),
          m_rightParser(std::move(rightParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        auto context = SubParserStream<Stream>(source);
        if(m_leftParser.Read(context) && m_rightParser.Read(context, value)) {
          context.Accept();
          return true;
        }
        return false;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        auto context = SubParserStream<Stream>(source);
        if(m_leftParser.Read(context) && m_rightParser.Read(context)) {
          context.Accept();
          return true;
        }
        return false;
      }

    private:
      LeftParser m_leftParser;
      RightParser m_rightParser;
  };

  template<typename L, typename R>
  class ConcatenateParser<L, R, std::enable_if_t<
      !std::is_same_v<parser_result_t<L>, NullType> &&
      std::is_same_v<parser_result_t<R>, NullType>>> :
      public BaseConcatenateParser {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = parser_result_t<LeftParser>;

      ConcatenateParser(LeftParser leftParser, RightParser rightParser)
        : m_leftParser(std::move(leftParser)),
          m_rightParser(std::move(rightParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        auto context = SubParserStream<Stream>(source);
        if(m_leftParser.Read(context, value) && m_rightParser.Read(context)) {
          context.Accept();
          return true;
        }
        return false;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        auto context = SubParserStream<Stream>(source);
        if(m_leftParser.Read(context) && m_rightParser.Read(context)) {
          context.Accept();
          return true;
        }
        return false;
      }

    private:
      LeftParser m_leftParser;
      RightParser m_rightParser;
  };

  template<typename L, typename R>
  class ConcatenateParser<L, R, std::enable_if_t<
      !std::is_base_of_v<BaseConcatenateParser, L> &&
      !std::is_same_v<parser_result_t<L>, NullType> &&
      !std::is_same_v<parser_result_t<R>, NullType>>> :
      public BaseConcatenateParser {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = std::tuple<parser_result_t<LeftParser>,
        parser_result_t<RightParser>>;

      ConcatenateParser(LeftParser leftParser, RightParser rightParser)
        : m_leftParser(std::move(leftParser)),
          m_rightParser(std::move(rightParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        auto context = SubParserStream<Stream>(source);
        auto leftValue = parser_result_t<LeftParser>();
        if(!m_leftParser.Read(context, leftValue)) {
          return false;
        }
        auto rightValue = parser_result_t<RightParser>();
        if(!m_rightParser.Read(context, rightValue)) {
          return false;
        }
        context.Accept();
        value = std::make_tuple(std::move(leftValue), std::move(rightValue));
        return true;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        auto context = SubParserStream<Stream>(source);
        if(m_leftParser.Read(context) && m_rightParser.Read(context)) {
          context.Accept();
          return true;
        }
        return false;
      }

    private:
      LeftParser m_leftParser;
      RightParser m_rightParser;
  };

  template<typename L, typename R>
  class ConcatenateParser<L, R, std::enable_if_t<
      std::is_base_of_v<BaseConcatenateParser, L> &&
      !std::is_same_v<parser_result_t<L>, NullType> &&
      !std::is_same_v<parser_result_t<R>, NullType>>> :
      public BaseConcatenateParser {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = typename Details::ConcatenateResult<
        parser_result_t<LeftParser>, parser_result_t<RightParser>>::type;

      ConcatenateParser(LeftParser leftParser, RightParser rightParser)
        : m_leftParser(std::move(leftParser)),
          m_rightParser(std::move(rightParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        auto context = SubParserStream<Stream>(source);
        auto leftValue = parser_result_t<LeftParser>();
        if(!m_leftParser.Read(context, leftValue)) {
          return false;
        }
        auto rightValue = parser_result_t<RightParser>();
        if(!m_rightParser.Read(context, rightValue)) {
          return false;
        }
        context.Accept();
        value = Details::TupleCat(std::move(leftValue), std::move(rightValue));
        return true;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        auto context = SubParserStream<Stream>(source);
        if(m_leftParser.Read(context) && m_rightParser.Read(context)) {
          context.Accept();
          return true;
        }
        return false;
      }

    private:
      LeftParser m_leftParser;
      RightParser m_rightParser;
  };

  template<typename L, typename R>
  ConcatenateParser(L, R) -> ConcatenateParser<to_parser_t<L>, to_parser_t<R>>;
}

#endif
