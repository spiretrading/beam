#ifndef BEAM_ORPARSER_HPP
#define BEAM_ORPARSER_HPP
#include <type_traits>
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Traits.hpp"

namespace Beam::Parsers {
namespace Details {
  template<typename T1, typename T2, typename Enabled = void>
  struct OrResult {};

  template<typename T1, typename T2>
  struct OrResult<T1, T2, std::enable_if_t<std::is_same_v<T1, T2>>> {
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
             a) NullType if both L and R are NullType Parsers.
             b) A boost::variant of NullType and L's Result if only R is a
                NullType Parser.
             c) A boost::variant of R's Result and NullType if only L is a
                NullType Parser.
             d) An boost::variant if both L's Result and R's Result if both
                parsers are not NullType.
      \tparam L The parser that must match to the left.
      \tparam R The parser that must match to the right.
   */
  template<typename L, typename R, typename Enabled>
  class OrParser {
    public:

      //! The parser that must match to the left.
      using LeftParser = L;

      //! The parser that must match to the right.
      using RightParser = R;
  };

  struct BaseOrParser {};

  template<typename L, typename R>
  class OrParser<L, R, std::enable_if_t<
      std::is_same_v<parser_result_t<L>, NullType> &&
      std::is_same_v<parser_result_t<R>, NullType>>> : public BaseOrParser {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = NullType;

      OrParser(LeftParser leftParser, RightParser rightParser)
        : m_leftParser(std::move(leftParser)),
          m_rightParser(std::move(rightParser)) {}

      template<typename Stream>
      bool Read(Stream& source) const {
        {
          auto context = SubParserStream<Stream>(source);
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

  template<typename L, typename R>
  class OrParser<L, R, std::enable_if_t<
      std::is_same_v<parser_result_t<L>, NullType> &&
      !std::is_same_v<parser_result_t<R>, NullType>>> : public BaseOrParser {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = boost::optional<parser_result_t<RightParser>>;

      OrParser(LeftParser leftParser, RightParser rightParser)
        : m_leftParser(std::move(leftParser)),
          m_rightParser(std::move(rightParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        {
          auto context = SubParserStream<Stream>(source);
          if(m_leftParser.Read(context)) {
            context.Accept();
            value = Result();
            return true;
          }
        }
        auto subValue = parser_result_t<RightParser>();
        if(m_rightParser.Read(source, subValue)) {
          value = std::move(subValue);
          return true;
        }
        return false;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        {
          auto context = SubParserStream<Stream>(source);
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

  template<typename L, typename R>
  class OrParser<L, R, std::enable_if_t<
      !std::is_same_v<parser_result_t<L>, NullType> &&
      std::is_same_v<parser_result_t<R>, NullType>>> : public BaseOrParser {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = boost::optional<parser_result_t<LeftParser>>;

      OrParser(LeftParser leftParser, RightParser rightParser)
        : m_leftParser(std::move(leftParser)),
          m_rightParser(std::move(rightParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        {
          auto context = SubParserStream<Stream>(source);
          auto subValue = parser_result_t<LeftParser>();
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

      template<typename Stream>
      bool Read(Stream& source) const {
        {
          auto context = SubParserStream<Stream>(source);
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

  template<typename L, typename R>
  class OrParser<L, R, std::enable_if_t<!std::is_base_of_v<BaseOrParser, L> &&
      !std::is_same_v<parser_result_t<L>, NullType> &&
      !std::is_same_v<parser_result_t<R>, NullType>>> : public BaseOrParser {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = std::conditional_t<std::is_same_v<
        parser_result_t<LeftParser>, parser_result_t<RightParser>>,
        parser_result_t<LeftParser>, boost::variant<parser_result_t<LeftParser>,
        parser_result_t<RightParser>>>;

      OrParser(LeftParser leftParser, RightParser rightParser)
        : m_leftParser(std::move(leftParser)),
          m_rightParser(std::move(rightParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        {
          auto context = SubParserStream<Stream>(source);
          auto subValue = parser_result_t<LeftParser>();
          if(m_leftParser.Read(context, subValue)) {
            context.Accept();
            value = std::move(subValue);
            return true;
          }
        }
        auto subValue = parser_result_t<RightParser>();
        if(m_rightParser.Read(source, subValue)) {
          value = std::move(subValue);
          return true;
        }
        return false;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        {
          auto context = SubParserStream<Stream>(source);
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

  template<typename L, typename R>
  class OrParser<L, R, std::enable_if_t<std::is_base_of_v<BaseOrParser, L> &&
      !std::is_same_v<parser_result_t<L>, NullType> &&
      !std::is_same_v<parser_result_t<R>, NullType>>> : public BaseOrParser {
    public:
      using LeftParser = L;
      using RightParser = R;
      using Result = typename Details::OrResult<parser_result_t<L>,
        parser_result_t<R>>::type;

      OrParser(LeftParser leftParser, RightParser rightParser)
        : m_leftParser(std::move(leftParser)),
          m_rightParser(std::move(rightParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        {
          auto context = SubParserStream<Stream>(source);
          auto subValue = parser_result_t<LeftParser>();
          if(m_leftParser.Read(context, subValue)) {
            context.Accept();
            value = std::move(subValue);
            return true;
          }
        }
        auto subValue = parser_result_t<RightParser>();
        if(m_rightParser.Read(source, subValue)) {
          value = std::move(subValue);
          return true;
        }
        return false;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        {
          auto context = SubParserStream<Stream>(source);
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

  template<typename L, typename R>
  OrParser(L, R) -> OrParser<to_parser_t<L>, to_parser_t<R>>;
}

#endif
