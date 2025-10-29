#ifndef BEAM_NONE_PARSER_HPP
#define BEAM_NONE_PARSER_HPP
#include "Beam/Parsers/Parser.hpp"

namespace Beam {

  /**
   * A parser that never results in a value.
   * @tparam R The type to evaluate to.
   */
  template<typename R>
  class NoneParser {
    public:
      using Result = R;
  };

  template<typename R> requires std::same_as<R, void>
  class NoneParser<R> {
    public:
      using Result = void;

      template<IsParserStream S>
      bool read(S& source) const {
        return false;
      }
  };

  template<typename R> requires(!std::same_as<R, void>)
  class NoneParser<R> {
    public:
      using Result = R;

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        return false;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        return false;
      }
  };

  /** A global instance of a NoneParser for convenience. */
  template<typename R = void>
  constexpr auto none_p = NoneParser<R>();
}

#endif
