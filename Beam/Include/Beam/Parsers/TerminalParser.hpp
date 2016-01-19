#ifndef BEAM_TERMINALPARSER_HPP
#define BEAM_TERMINALPARSER_HPP
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/ParserStream.hpp"

namespace Beam {
namespace Parsers {

  /*! \class TerminalParser
      \brief Matches a single character.
   */
  class TerminalParser : public ParserOperators {
    public:
      typedef NullType Result;

      //! Constructs a TerminalParser.
      /*!
        \param value The value to match.
      */
      TerminalParser(char value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);

    private:
      char m_value;
  };

  /*! \class ConstTerminalParser
      \brief Matches a single compile time character.
   */
  template<char C>
  class ConstTerminalParser : public ParserOperators {
    public:
      typedef NullType Result;

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);
  };

  inline TerminalParser::TerminalParser(char value)
      : m_value(value) {}

  template<typename ParserStreamType>
  bool TerminalParser::Read(ParserStreamType& source) {
    if(!source.Read()) {
      return false;
    }
    if(source.GetChar() != m_value) {
      source.Undo();
      return false;
    }
    return true;
  }

  template<char C>
  template<typename ParserStreamType>
  bool ConstTerminalParser<C>::Read(ParserStreamType& source) {
    if(!source.Read()) {
      return false;
    }
    if(source.GetChar() != C) {
      source.Undo();
      return false;
    }
    return true;
  }
}
}

#endif
