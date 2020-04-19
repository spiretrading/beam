#ifndef BEAM_CONVERSIONPARSER_HPP
#define BEAM_CONVERSIONPARSER_HPP
#include <type_traits>
#include <boost/utility/declval.hpp>
#include "Beam/Parsers/Operators.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam {
namespace Parsers {
namespace Details {
  template<typename SubParserType, typename ConversionFunctionType>
  struct GetConversionResultType {
    using Result = decltype(boost::declval<ConversionFunctionType>()(
      boost::declval<typename SubParserType::Result>()));
    using type = typename std::decay<Result>::type;
  };

  template<typename ConversionFunctionType>
  struct GetNullConversionResultType {
    using type = decltype(boost::declval<ConversionFunctionType>()());
  };

  template<typename SubParserType, typename ConversionFunctionType, bool Test>
  struct IsConversionVoid {};

  template<typename SubParserType, typename ConversionFunctionType>
  struct IsConversionVoid<SubParserType, ConversionFunctionType, false> {
    static const bool value =
      std::is_same<typename GetConversionResultType<SubParserType,
      ConversionFunctionType>::type, void>::value;
  };

  template<typename SubParserType, typename ConversionFunctionType>
  struct IsConversionVoid<SubParserType, ConversionFunctionType, true> {
    static const bool value =
      std::is_same<typename GetNullConversionResultType<
      ConversionFunctionType>::type, void>::value;
  };

  template<typename SubParserType, typename ConversionFunctionType>
  struct NoNullConversion {
    static const bool value =
      !std::is_same<typename SubParserType::Result, NullType>::value &&
      !IsConversionVoid<SubParserType, ConversionFunctionType,
      std::is_same<typename SubParserType::Result, NullType>::value>::value;
  };

  template<typename SubParserType, typename ConversionFunctionType>
  struct IsSuppressingSubParser {
    static const bool value =
      !std::is_same<typename SubParserType::Result, NullType>::value &&
      IsConversionVoid<SubParserType, ConversionFunctionType,
      std::is_same<typename SubParserType::Result, NullType>::value>::value;
  };

  template<typename SubParserType, typename ConversionFunctionType>
  struct IsExtendingSubParser {
    static const bool value =
      std::is_same<typename SubParserType::Result, NullType>::value &&
      !IsConversionVoid<SubParserType, ConversionFunctionType,
      std::is_same<typename SubParserType::Result, NullType>::value>::value;
  };

  template<typename SubParserType, typename ConversionFunctionType>
  struct IsSuppressingAll {
    static const bool value =
      std::is_same<typename SubParserType::Result, NullType>::value &&
      IsConversionVoid<SubParserType, ConversionFunctionType,
      std::is_same<typename SubParserType::Result, NullType>::value>::value;
  };
}

  template<typename SubParserType, typename ConversionFunctionType,
    typename Enabled>
  class ConversionParser {};

  template<typename SubParserType, typename ConversionFunctionType>
  class ConversionParser<SubParserType, ConversionFunctionType,
      typename std::enable_if<Details::NoNullConversion<SubParserType,
      ConversionFunctionType>::value || Details::IsSuppressingSubParser<
      SubParserType, ConversionFunctionType>::value>::type> :
      public ParserOperators {
    public:
      typedef SubParserType SubParser;
      typedef ConversionFunctionType ConversionFunction;
      typedef typename Details::GetConversionResultType<
        SubParser, ConversionFunction>::type Result;

      ConversionParser(const SubParser& subParser,
          ConversionFunction conversionFunction)
          : m_subParser(subParser),
            m_conversionFunction(conversionFunction) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        typename SubParser::Result subValue;
        if(!m_subParser.Read(source, subValue)) {
          return false;
        }
        value = m_conversionFunction(subValue);
        return true;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        typename SubParser::Result subValue;
        if(!m_subParser.Read(source, subValue)) {
          return false;
        }
        m_conversionFunction(subValue);
        return true;
      }

    private:
      SubParser m_subParser;
      ConversionFunction m_conversionFunction;
  };

  template<typename SubParserType, typename ConversionFunctionType>
  class ConversionParser<SubParserType, ConversionFunctionType,
      typename std::enable_if<Details::IsExtendingSubParser<SubParserType,
      ConversionFunctionType>::value || Details::IsSuppressingAll<SubParserType,
      ConversionFunctionType>::value>::type> : public ParserOperators {
    public:
      typedef SubParserType SubParser;
      typedef ConversionFunctionType ConversionFunction;
      typedef typename Details::GetNullConversionResultType<
        ConversionFunction>::type Result;

      ConversionParser(const SubParser& subParser,
          ConversionFunction conversionFunction)
          : m_subParser(subParser),
            m_conversionFunction(conversionFunction) {}

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value) {
        if(!m_subParser.Read(source)) {
          return false;
        }
        value = m_conversionFunction();
        return true;
      }

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source) {
        if(!m_subParser.Read(source)) {
          return false;
        }
        m_conversionFunction();
        return true;
      }

    private:
      SubParser m_subParser;
      ConversionFunction m_conversionFunction;
  };

  template<typename SubParser, typename F>
  ConversionParser<typename GetParserType<SubParser>::type, F> Convert(
      const SubParser& subParser, F f) {
    return ConversionParser<typename GetParserType<SubParser>::type, F>(
      subParser, f);
  }
}
}

#endif
