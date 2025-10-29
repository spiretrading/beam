#ifndef BEAM_EPSILON_PARSER_HPP
#define BEAM_EPSILON_PARSER_HPP
#include "Beam/Parsers/Parser.hpp"

namespace Beam {

  /**
   * Does not perform any read operations and always returns <code>true</code>.
   */
  class EpsilonParser {
    public:
      using Result = void;

      template<IsParserStream S>
      bool read(S& source) const;
  };

  /** An instance of an EpsilonParser. */
  inline const auto eps_p = EpsilonParser();

  template<IsParserStream S>
  bool EpsilonParser::read(S& source) const {
    return true;
  }
}

#endif
