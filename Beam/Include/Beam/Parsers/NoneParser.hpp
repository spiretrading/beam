#ifndef BEAM_NONE_PARSER_HPP
#define BEAM_NONE_PARSER_HPP
#include "Beam/Parsers/Parsers.hpp"

namespace Beam::Parsers {

  /**
   * A parser that never results in a value.
   * @param <R> The type to evaluate to.
   */
  template<typename R>
  class NoneParser {
    public:
      using Result = R;

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;
  };

  template<typename R>
  template<typename Stream>
  bool NoneParser<R>::Read(Stream& source, Result& value) const {
    return false;
  }

  template<typename R>
  template<typename Stream>
  bool NoneParser<R>::Read(Stream& source) const {
    return false;
  }
}

#endif
