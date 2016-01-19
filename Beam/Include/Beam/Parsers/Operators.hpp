#ifndef BEAM_PARSEROPERATORS_HPP
#define BEAM_PARSEROPERATORS_HPP
#include <type_traits>
#include "Beam/Parsers/ConcatenateParser.hpp"
#include "Beam/Parsers/DifferenceParser.hpp"
#include "Beam/Parsers/ListParser.hpp"
#include "Beam/Parsers/OrParser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/PlusParser.hpp"
#include "Beam/Parsers/StarParser.hpp"
#include "Beam/Parsers/SymbolParser.hpp"
#include "Beam/Parsers/TerminalParser.hpp"

namespace Beam {
namespace Parsers {
  template<>
  struct GetParserType<char> {
    typedef TerminalParser type;
  };

  template<>
  struct GetParserType<const char*> {
    typedef SymbolParser type;
  };

  template<>
  struct GetParserType<std::string> {
    typedef SymbolParser type;
  };

  template<std::size_t N>
  struct GetParserType<const char[N]> {
    typedef SymbolParser type;
  };

  template<std::size_t N>
  struct GetParserType<char[N]> {
    typedef SymbolParser type;
  };

  template<typename T>
  struct IsParser {
    static const bool value = std::is_base_of<ParserOperators,
      typename GetParserType<T>::type>::value;
  };

  template<typename LeftParser, typename RightParser>
  typename std::enable_if<
      IsParser<LeftParser>::value && IsParser<RightParser>::value,
      ConcatenateParser<typename GetParserType<LeftParser>::type,
      typename GetParserType<RightParser>::type>>::type operator >>(
      const LeftParser& leftParser, const RightParser& rightParser) {
    return ConcatenateParser<typename GetParserType<LeftParser>::type,
      typename GetParserType<RightParser>::type>(leftParser, rightParser);
  }

  template<typename LeftParser, typename RightParser>
  typename std::enable_if<
      IsParser<LeftParser>::value && IsParser<RightParser>::value,
      DifferenceParser<typename GetParserType<LeftParser>::type,
      typename GetParserType<RightParser>::type>>::type operator -(
      const LeftParser& leftParser, const RightParser& rightParser) {
    return DifferenceParser<typename GetParserType<LeftParser>::type,
      typename GetParserType<RightParser>::type>(leftParser, rightParser);
  }

  template<typename LeftParser, typename RightParser>
  typename std::enable_if<
      IsParser<LeftParser>::value && IsParser<RightParser>::value,
      OrParser<typename GetParserType<LeftParser>::type,
      typename GetParserType<RightParser>::type>>::type operator |(
      const LeftParser& leftParser, const RightParser& rightParser) {
    return OrParser<typename GetParserType<LeftParser>::type,
      typename GetParserType<RightParser>::type>(leftParser, rightParser);
  }

  template<typename SubParser>
  typename std::enable_if<IsParser<SubParser>::value,
      StarParser<typename GetParserType<SubParser>::type>>::type operator *(
      const SubParser& subParser) {
    return StarParser<typename GetParserType<SubParser>::type>(subParser);
  }

  template<typename SubParser>
  typename std::enable_if<IsParser<SubParser>::value,
      PlusParser<typename GetParserType<SubParser>::type>>::type operator +(
      const SubParser& subParser) {
    return PlusParser<typename GetParserType<SubParser>::type>(subParser);
  }
}
}

#endif
