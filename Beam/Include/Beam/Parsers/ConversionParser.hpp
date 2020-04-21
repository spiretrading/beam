#ifndef BEAM_CONVERSIONPARSER_HPP
#define BEAM_CONVERSIONPARSER_HPP
#include <type_traits>
#include <utility>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/Traits.hpp"
#include "Beam/Utilities/NullType.hpp"

namespace Beam::Parsers {
namespace Details {
  template<typename P, typename C>
  struct GetConversionResultType {
    using type = std::decay_t<decltype(
      std::declval<C>()(std::declval<parser_result_t<P>>()))>;
  };

  template<typename C>
  struct GetNullConversionResultType {
    using type = std::decay_t<decltype(std::declval<C>()())>;
  };

  template<typename P, typename C, bool T>
  struct IsConversionVoid {};

  template<typename P, typename C>
  struct IsConversionVoid<P, C, false> {
    static constexpr bool value = std::is_same_v<
      typename GetConversionResultType<P, C>::type, void>;
  };

  template<typename P, typename C>
  struct IsConversionVoid<P, C, true> {
    static constexpr bool value = std::is_same_v<
      typename GetNullConversionResultType<C>::type, void>;
  };

  template<typename P, typename C>
  struct NoNullConversion {
    static constexpr bool value =
      !std::is_same_v<parser_result_t<P>, NullType> && !IsConversionVoid<P, C,
      std::is_same_v<parser_result_t<P>, NullType>>::value;
  };

  template<typename P, typename C>
  struct IsSuppressingSubParser {
    static constexpr bool value =
      !std::is_same_v<parser_result_t<P>, NullType> && IsConversionVoid<P, C,
      std::is_same_v<parser_result_t<P>, NullType>>::value;
  };

  template<typename P, typename C>
  struct IsExtendingSubParser {
    static constexpr bool value =
      std::is_same_v<parser_result_t<P>, NullType> && !IsConversionVoid<P, C,
      std::is_same_v<parser_result_t<P>, NullType>>::value;
  };

  template<typename P, typename C>
  struct IsSuppressingAll {
    static constexpr bool value =
      std::is_same_v<parser_result_t<P>, NullType> && IsConversionVoid<P, C,
      std::is_same_v<parser_result_t<P>, NullType>>::value;
  };
}

  template<typename P, typename C, typename E>
  class ConversionParser {};

  template<typename P, typename C>
  class ConversionParser<P, C, std::enable_if_t<Details::NoNullConversion<P,
      C>::value || Details::IsSuppressingSubParser<P, C>::value>> {
    public:
      using SubParser = P;
      using ConversionFunction = C;
      using Result = typename Details::GetConversionResultType<
        SubParser, ConversionFunction>::type;

      ConversionParser(SubParser subParser,
        ConversionFunction conversionFunction)
        : m_subParser(std::move(subParser)),
          m_conversionFunction(std::move(conversionFunction)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        auto subValue = parser_result_t<SubParser>();
        if(!m_subParser.Read(source, subValue)) {
          return false;
        }
        value = m_conversionFunction(std::move(subValue));
        return true;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        auto subValue = parser_result_t<SubParser>();
        if(!m_subParser.Read(source, subValue)) {
          return false;
        }
        m_conversionFunction(std::move(subValue));
        return true;
      }

    private:
      SubParser m_subParser;
      ConversionFunction m_conversionFunction;
  };

  template<typename P, typename C>
  class ConversionParser<P, C, std::enable_if_t<
      Details::IsExtendingSubParser<P, C>::value ||
      Details::IsSuppressingAll<P, C>::value>> {
    public:
      using SubParser = P;
      using ConversionFunction = C;
      using Result = typename Details::GetNullConversionResultType<
        ConversionFunction>::type;

      ConversionParser(SubParser subParser,
        ConversionFunction conversionFunction)
        : m_subParser(std::move(subParser)),
          m_conversionFunction(std::move(conversionFunction)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        if(!m_subParser.Read(source)) {
          return false;
        }
        value = m_conversionFunction();
        return true;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
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

  template<typename P, typename C>
  ConversionParser(P, C) -> ConversionParser<to_parser_t<P>, C>;

  template<typename Parser, typename F>
  auto Convert(Parser subParser, F f) {
    return ConversionParser(std::move(subParser), std::move(f));
  }

  template<typename T, typename SubParser>
  auto Cast(SubParser subParser) {
    return ConversionParser(std::move(subParser),
      [] (auto&& value) {
        return static_cast<T>(std::forward<decltype(value)>(value));
      });
  }
}

#endif
