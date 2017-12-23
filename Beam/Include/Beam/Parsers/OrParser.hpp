#ifndef BEAM_ORPARSER_HPP
#define BEAM_ORPARSER_HPP
#include <type_traits>
#include <boost/variant/variant.hpp>
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam {
namespace Parsers {
namespace Details {
  template<typename T1, typename T2, typename Enabled = void>
  struct OrResult {};

  template<typename T1, typename T2>
  struct OrResult<T1, T2,
      typename std::enable_if<std::is_same<T1, T2>::value>::type> {
    using type = T1;
  };

  template<typename... L, typename R>
  struct OrResult<boost::variant<L...>, R> {
    using type = boost::variant<L..., R>;
  };
}

  /*! \class OrParser
      \brief Parses a match if either of its two sub-parsers match.
             The result of the parsing is one of:
             a) NullType if both LeftParserType and RightParserType are NullType
                Parsers.
             b) A boost::variant of NullType and LeftParserType's Result if only
                RightParserType is a NullType Parser.
             c) A boost::variant of RightParserType's Result and NullType if
                only LeftParserType is a NullType Parser.
             d) An boost::variant if both LeftParserType's Result and
                RightParserType's Result if both parsers are not NullType.
      \tparam LeftParserType The parser that must match to the left.
      \tparam RightParserType The parser that must match to the right.
   */
  template<typename LeftParserType, typename RightParserType, typename Enabled>
  class OrParser {
    public:

      //! The parser that must match to the left.
      using LeftParser = LeftParserType;

      //! The parser that must match to the right.
      using RightParser = RightParserType;
  };

  struct BaseOrParser {};

  template<typename LeftParserType, typename RightParserType>
  class OrParser<LeftParserType, RightParserType,
      typename std::enable_if<
      std::is_same<typename LeftParserType::Result, NullType>::value &&
      std::is_same<typename RightParserType::Result, NullType>::value>::type> :
      public ParserOperators, public BaseOrParser {
    public:
      using LeftParser = LeftParserType;
      using RightParser = RightParserType;
      using Result = NullType;

      OrParser(const LeftParser& leftParser, const RightParser& rightParser)
          : m_leftParser(leftParser),
            m_rightParser(rightParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        {
          SubParserStream<ParserStreamType> context(source);
          if(m_leftParser.Read(context)) {
            context.Accept();
            return true;
          }
        }
        return m_rightParser.Read(source);
      }

    private:
      LeftParser m_leftParser;
      RightParser m_rightParser;
  };

  template<typename LeftParserType, typename RightParserType>
  class OrParser<LeftParserType, RightParserType,
      typename std::enable_if<
      std::is_same<typename LeftParserType::Result, NullType>::value &&
      !std::is_same<typename RightParserType::Result, NullType>::value>::type> :
      public ParserOperators, public BaseOrParser {
    public:
      using LeftParser = LeftParserType;
      using RightParser = RightParserType;
      using Result = boost::optional<typename RightParser::Result>;

      OrParser(const LeftParser& leftParser, const RightParser& rightParser)
          : m_leftParser(leftParser),
            m_rightParser(rightParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        {
          SubParserStream<ParserStreamType> context(source);
          if(m_leftParser.Read(context)) {
            context.Accept();
            value = Result();
            return true;
          }
        }
        typename RightParser::Result subValue;
        if(m_rightParser.Read(source, subValue)) {
          value = std::move(subValue);
          return true;
        }
        return false;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        {
          SubParserStream<ParserStreamType> context(source);
          if(m_leftParser.Read(context)) {
            context.Accept();
            return true;
          }
        }
        return m_rightParser.Read(source);
      }

    private:
      LeftParser m_leftParser;
      RightParser m_rightParser;
  };

  template<typename LeftParserType, typename RightParserType>
  class OrParser<LeftParserType, RightParserType,
      typename std::enable_if<
      !std::is_same<typename LeftParserType::Result, NullType>::value &&
      std::is_same<typename RightParserType::Result, NullType>::value>::type> :
      public ParserOperators, public BaseOrParser {
    public:
      using LeftParser = LeftParserType;
      using RightParser = RightParserType;
      using Result = boost::optional<typename LeftParser::Result>;

      OrParser(const LeftParser& leftParser, const RightParser& rightParser)
          : m_leftParser(leftParser),
            m_rightParser(rightParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        {
          SubParserStream<ParserStreamType> context(source);
          typename LeftParser::Result subValue;
          if(m_leftParser.Read(context, subValue)) {
            context.Accept();
            value = std::move(subValue);
            return true;
          }
        }
        if(m_rightParser.Read(source)) {
          value = Result();
          return true;
        }
        return false;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        {
          SubParserStream<ParserStreamType> context(source);
          if(m_leftParser.Read(context)) {
            context.Accept();
            return true;
          }
        }
        return m_rightParser.Read(source);
      }

    private:
      LeftParser m_leftParser;
      RightParser m_rightParser;
  };

  template<typename LeftParserType, typename RightParserType>
  class OrParser<LeftParserType, RightParserType,
      typename std::enable_if<
      !std::is_base_of<BaseOrParser, LeftParserType>::value &&
      !std::is_same<typename LeftParserType::Result, NullType>::value &&
      !std::is_same<typename RightParserType::Result, NullType>::value>::type> :
      public ParserOperators, public BaseOrParser {
    public:
      using LeftParser = LeftParserType;
      using RightParser = RightParserType;
      using Result = typename std::conditional<
        std::is_same<typename LeftParser::Result,
        typename RightParser::Result>::value, typename LeftParser::Result,
        boost::variant<typename LeftParser::Result,
        typename RightParser::Result>>::type;

      OrParser(const LeftParser& leftParser, const RightParser& rightParser)
          : m_leftParser(leftParser),
            m_rightParser(rightParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        {
          SubParserStream<ParserStreamType> context(source);
          typename LeftParser::Result subValue;
          if(m_leftParser.Read(context, subValue)) {
            context.Accept();
            value = std::move(subValue);
            return true;
          }
        }
        typename RightParser::Result subValue;
        if(m_rightParser.Read(source, subValue)) {
          value = std::move(subValue);
          return true;
        }
        return false;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        {
          SubParserStream<ParserStreamType> context(source);
          if(m_leftParser.Read(context)) {
            context.Accept();
            return true;
          }
        }
        return m_rightParser.Read(source);
      }

    private:
      LeftParser m_leftParser;
      RightParser m_rightParser;
  };

  template<typename LeftParserType, typename RightParserType>
  class OrParser<LeftParserType, RightParserType,
      typename std::enable_if<
      std::is_base_of<BaseOrParser, LeftParserType>::value &&
      !std::is_same<typename LeftParserType::Result, NullType>::value &&
      !std::is_same<typename RightParserType::Result, NullType>::value>::type> :
      public ParserOperators, public BaseOrParser {
    public:
      using LeftParser = LeftParserType;
      using RightParser = RightParserType;
      using Result = typename Details::OrResult<typename LeftParserType::Result,
        typename RightParserType::Result>::type;

      OrParser(const LeftParser& leftParser, const RightParser& rightParser)
          : m_leftParser(leftParser),
            m_rightParser(rightParser) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        {
          SubParserStream<ParserStreamType> context(source);
          typename LeftParser::Result subValue;
          if(m_leftParser.Read(context, subValue)) {
            context.Accept();
            value = std::move(subValue);
            return true;
          }
        }
        typename RightParser::Result subValue;
        if(m_rightParser.Read(source, subValue)) {
          value = std::move(subValue);
          return true;
        }
        return false;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        {
          SubParserStream<ParserStreamType> context(source);
          if(m_leftParser.Read(context)) {
            context.Accept();
            return true;
          }
        }
        return m_rightParser.Read(source);
      }

    private:
      LeftParser m_leftParser;
      RightParser m_rightParser;
  };
}
}

#endif
