#ifndef BEAM_CONCATENATEPARSER_HPP
#define BEAM_CONCATENATEPARSER_HPP
#include <tuple>
#include <type_traits>
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam {
namespace Parsers {
namespace Details {
  template<typename T1, typename T2>
  struct ConcatenateResult {
    typedef std::tuple<T1, T2> type;
  };

  template<typename T1>
  struct ConcatenateResult<T1, NullType> {
    typedef T1 type;
  };

  template<typename T1>
  struct ConcatenateResult<NullType, T1> {
    typedef T1 type;
  };

  template<typename... T1, typename... T2>
  struct ConcatenateResult<std::tuple<T1...>, std::tuple<T2...>> {
    typedef std::tuple<T1..., T2...> type;
  };

  template<typename... T1>
  struct ConcatenateResult<std::tuple<T1...>, NullType> {
    typedef std::tuple<T1...> type;
  };

  template<typename... T1>
  struct ConcatenateResult<NullType, std::tuple<T1...>> {
    typedef std::tuple<T1...> type;
  };

  template<typename T1, typename... T2>
  struct ConcatenateResult<T1, std::tuple<T2...>> {
    typedef std::tuple<T1, T2...> type;
  };

  template<typename T1, typename... T2>
  struct ConcatenateResult<std::tuple<T2...>, T1> {
    typedef std::tuple<T2..., T1> type;
  };

  template<typename... T1, typename... T2>
  std::tuple<T1..., T2...> TupleCat(std::tuple<T1...>&& t1,
      std::tuple<T2...>&& t2) {
    return std::tuple_cat(std::move(t1), std::move(t2));
  }

  template<typename T1, typename... T2>
  std::tuple<T1, T2...> TupleCat(T1&& t1, std::tuple<T2...>&& t2) {
    return std::tuple_cat(std::make_tuple(std::move(t1)), std::move(t2));
  }

  template<typename T1, typename... T2>
  std::tuple<T2..., T1> TupleCat(std::tuple<T2...>&& t2, T1&& t1) {
    return std::tuple_cat(std::move(t2), std::make_tuple(std::move(t1)));
  }

  template<typename T1, typename T2>
  std::tuple<T1, T2> TupleCat(T1&& t1, T2&& t2) {
    return std::make_tuple(std::move(t1), std::move(t2));
  }
}

  /*! \class ConcatenateParser
      \brief Concatenates two parsers so that they must both match in order.
             The result of the parsing is one of:
             a) NullType if both LeftParserType and RightParserType are NullType
                Parsers.
             b) LeftParserType if only RightParserType is a NullType Parser.
             c) RightParserType if only LeftParserType is a NullType Parser.
             d) An std::tuple of both LeftParserType's Result and
                RightParserType's Result.
      \tparam LeftParserType The parser that must match to the left.
      \tparam RightParserType The parser that must match to the right.
   */
  template<typename LeftParserType, typename RightParserType, typename Enabled>
  class ConcatenateParser {
    public:

      //! The parser that must match to the left.
      typedef LeftParserType LeftParser;

      //! The parser that must match to the right.
      typedef RightParserType RightParser;
  };

  struct BaseConcatenateParser {};

  template<typename LeftParserType, typename RightParserType>
  class ConcatenateParser<LeftParserType, RightParserType,
      typename std::enable_if<
      std::is_same<typename LeftParserType::Result, NullType>::value &&
      std::is_same<typename RightParserType::Result, NullType>::value>::type> :
      public ParserOperators, public BaseConcatenateParser {
    public:
      typedef LeftParserType LeftParser;
      typedef RightParserType RightParser;
      typedef NullType Result;

      ConcatenateParser(const LeftParser& leftParser,
          const RightParser& rightParser)
          : m_leftParser(leftParser),
            m_rightParser(rightParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        SubParserStream<ParserStreamType> context(source);
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

  template<typename LeftParserType, typename RightParserType>
  class ConcatenateParser<LeftParserType, RightParserType,
      typename std::enable_if<
      std::is_same<typename LeftParserType::Result, NullType>::value &&
      !std::is_same<typename RightParserType::Result, NullType>::value>::type> :
      public ParserOperators, public BaseConcatenateParser {
    public:
      typedef LeftParserType LeftParser;
      typedef RightParserType RightParser;
      typedef typename RightParser::Result Result;

      ConcatenateParser(const LeftParser& leftParser,
          const RightParser& rightParser)
          : m_leftParser(leftParser),
            m_rightParser(rightParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        SubParserStream<ParserStreamType> context(source);
        if(m_leftParser.Read(context) && m_rightParser.Read(context, value)) {
          context.Accept();
          return true;
        }
        return false;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        SubParserStream<ParserStreamType> context(source);
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

  template<typename LeftParserType, typename RightParserType>
  class ConcatenateParser<LeftParserType, RightParserType,
      typename std::enable_if<
      !std::is_same<typename LeftParserType::Result, NullType>::value &&
      std::is_same<typename RightParserType::Result, NullType>::value>::type> :
      public ParserOperators, public BaseConcatenateParser {
    public:
      typedef LeftParserType LeftParser;
      typedef RightParserType RightParser;
      typedef typename LeftParser::Result Result;

      ConcatenateParser(const LeftParser& leftParser,
          const RightParser& rightParser)
          : m_leftParser(leftParser),
            m_rightParser(rightParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        SubParserStream<ParserStreamType> context(source);
        if(m_leftParser.Read(context, value) && m_rightParser.Read(context)) {
          context.Accept();
          return true;
        }
        return false;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        SubParserStream<ParserStreamType> context(source);
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

  template<typename LeftParserType, typename RightParserType>
  class ConcatenateParser<LeftParserType, RightParserType,
      typename std::enable_if<
      !std::is_base_of<BaseConcatenateParser, LeftParserType>::value &&
      !std::is_same<typename LeftParserType::Result, NullType>::value &&
      !std::is_same<typename RightParserType::Result, NullType>::value>::type> :
      public ParserOperators, public BaseConcatenateParser {
    public:
      typedef LeftParserType LeftParser;
      typedef RightParserType RightParser;
      typedef std::tuple<typename LeftParser::Result,
        typename RightParser::Result> Result;

      ConcatenateParser(const LeftParser& leftParser,
          const RightParser& rightParser)
          : m_leftParser(leftParser),
            m_rightParser(rightParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        SubParserStream<ParserStreamType> context(source);
        typename LeftParser::Result leftValue;
        if(!m_leftParser.Read(context, leftValue)) {
          return false;
        }
        typename RightParser::Result rightValue;
        if(!m_rightParser.Read(context, rightValue)) {
          return false;
        }
        context.Accept();
        value = std::make_tuple(leftValue, rightValue);
        return true;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        SubParserStream<ParserStreamType> context(source);
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

  template<typename LeftParserType, typename RightParserType>
  class ConcatenateParser<LeftParserType, RightParserType,
      typename std::enable_if<
      std::is_base_of<BaseConcatenateParser, LeftParserType>::value &&
      !std::is_same<typename LeftParserType::Result, NullType>::value &&
      !std::is_same<typename RightParserType::Result, NullType>::value>::type> :
      public ParserOperators, public BaseConcatenateParser {
    public:
      typedef LeftParserType LeftParser;
      typedef RightParserType RightParser;
      typedef typename Details::ConcatenateResult<
        typename LeftParserType::Result, typename RightParserType::Result>::type
        Result;

      ConcatenateParser(const LeftParser& leftParser,
          const RightParser& rightParser)
          : m_leftParser(leftParser),
            m_rightParser(rightParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        SubParserStream<ParserStreamType> context(source);
        typename LeftParser::Result leftValue;
        if(!m_leftParser.Read(context, leftValue)) {
          return false;
        }
        typename RightParser::Result rightValue;
        if(!m_rightParser.Read(context, rightValue)) {
          return false;
        }
        context.Accept();
        value = Details::TupleCat(std::move(leftValue), std::move(rightValue));
        return true;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        SubParserStream<ParserStreamType> context(source);
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
}
}

#endif
