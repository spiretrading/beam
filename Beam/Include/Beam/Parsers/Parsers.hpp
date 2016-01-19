#ifndef BEAM_PARSERS_HPP
#define BEAM_PARSERS_HPP

namespace Beam {
namespace Parsers {
  class AlphaParser;
  class AnyParser;
  template<typename ResultType> class BasicParser;
  class BlankParser;
  class BoolParser;
  template<typename LeftParserType, typename RightParserType,
    typename Enabled = void> class ConcatenateParser;
  template<typename SubParserType, typename ConversionFunctionType,
    typename Enabled = void> class ConversionParser;
  template<typename ResultType> class CustomParser;
  class DateTimeParser;
  template<typename FloatingType> class DecimalParser;
  template<typename LeftParserType, typename RightParserType>
    class DifferenceParser;
  class DigitParser;
  template<typename SubParserType> class DiscardParser;
  template<typename EnumeratorType> class EnumeratorParser;
  class EpsilonParser;
  template<typename ParserType, typename ResultType, typename ModifierType,
    typename Enabled = void> class ForListParser;
  template<typename IntegralType> class IntegralParser;
  template<typename ParserType> class ListParser;
  template<typename LeftParserType, typename RightParserType,
    typename Enabled = void> class OrParser;
  template<typename ResultType> struct Parser;
  class ParserException;
  template<typename ReaderType, typename ParserType> class ParserPublisher;
  struct ParserStream;
  template<typename SubParserType, typename Enabled = void> class PlusParser;
  template<typename IntegralType> class RationalParser;
  template<typename ReaderType> class ReaderParserStream;
  template<typename ResultType> class RuleParser;
  template<typename ParserType> class SequenceParser;
  class SkipSpaceParser;
  class SpaceParser;
  template<typename SubParserType, typename Enabled = void> class StarParser;
  class StringParser;
  template<typename ParserStreamType> class SubParserStream;
  class TerminalParser;
  class TimeDurationParser;
  template<typename SubParserType> class TokenParser;
  template<typename ResultType> class VirtualParser;
  template<typename ParserType, typename Enabled = void> class WrapperParser;
  class VirtualParserStream;
  template<typename ParserStreamType> class WrapperParserStream;
}
}

#endif
