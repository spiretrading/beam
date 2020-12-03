#ifndef BEAM_TERMINAL_PARSER_HPP
#define BEAM_TERMINAL_PARSER_HPP
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Utilities/NullType.hpp"

namespace Beam::Parsers {

  /** Matches a single character. */
  class TerminalParser {
    public:
      using Result = NullType;

      /**
       * Constructs a TerminalParser.
       * @param value The value to match.
       */
      TerminalParser(char value);

      template<typename Stream>
      bool Read(Stream& source) const;

    private:
      char m_value;
  };

  inline TerminalParser::TerminalParser(char value)
    : m_value(value) {}

  template<typename Stream>
  bool TerminalParser::Read(Stream& source) const {
    if(!source.Read()) {
      return false;
    }
    if(source.GetChar() != m_value) {
      source.Undo();
      return false;
    }
    return true;
  }
}

#endif
