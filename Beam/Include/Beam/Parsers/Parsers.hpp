#ifndef BEAM_PARSERS_HPP
#define BEAM_PARSERS_HPP

namespace Beam::Parsers {
  class AlphaParser;
  class AnyParser;
  class BlankParser;
  class BoolParser;
  template<typename L, typename R, typename E = void> class ConcatenateParser;
  template<typename P, typename F, typename E = void> class ConversionParser;
  template<typename F> class DecimalParser;
  template<typename L, typename R> class DifferenceParser;
  class DigitParser;
  template<typename P> class DiscardParser;
  template<typename E> class EnumeratorParser;
  class EpsilonParser;
  template<typename P, typename R, typename M, typename E = void>
    class ForListParser;
  template<typename I> class IntegralParser;
  template<typename P> class ListParser;
  template<typename R> class NoneParser;
  template<typename P> class NotParser;
  template<typename L, typename R, typename E = void> class OrParser;
  template<typename R> struct Parser;
  class ParserException;
  template<typename R, typename P> class ParserPublisher;
  struct ParserStream;
  template<typename P, typename E = void> class PlusParser;
  template<typename I> class RationalParser;
  template<typename R> class ReaderParserStream;
  template<typename R> class RuleParser;
  template<typename P> class SequenceParser;
  class SkipSpaceParser;
  class SpaceParser;
  template<typename P, typename E = void> class StarParser;
  class StringParser;
  template<typename P> class SubParserStream;
  class TerminalParser;
  template<typename P> class TokenParser;
  template<typename R> class VirtualParser;
  template<typename P, typename E = void> class WrapperParser;
  class VirtualParserStream;
  template<typename P> class WrapperParserStream;
}

#endif
