#ifndef BEAM_TERMINAL_PARSER_HPP
#define BEAM_TERMINAL_PARSER_HPP
#include "Beam/Parsers/Parser.hpp"

namespace Beam {

  /** Matches a single character. */
  class TerminalParser {
    public:
      using Result = void;

      /**
       * Constructs a TerminalParser.
       * @param value The value to match.
       */
      TerminalParser(char value) noexcept;

      template<IsParserStream S>
      bool read(S& source) const;

    private:
      char m_value;
  };

  inline TerminalParser::TerminalParser(char value) noexcept
    : m_value(value) {}

  template<IsParserStream S>
  bool TerminalParser::read(S& source) const {
    if(!source.read()) {
      return false;
    }
    if(source.peek() != m_value) {
      source.undo();
      return false;
    }
    return true;
  }
}

#endif
