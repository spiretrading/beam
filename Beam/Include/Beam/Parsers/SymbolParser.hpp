#ifndef BEAM_SYMBOLPARSER_HPP
#define BEAM_SYMBOLPARSER_HPP
#include <string>
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {
namespace Parsers {
namespace Details {
  template<typename T>
  struct SymbolConverter {
    T m_value;

    SymbolConverter(const T& value)
        : m_value(value) {}

    T operator ()() const {
      return m_value;
    }
  };
}

  /*! \class Symbol
      \brief Matches a symbol.
   */
  class SymbolParser : public ParserOperators {
    public:
      typedef NullType Result;

      //! Constructs a SymbolParser.
      /*!
        \param symbol The symbol to match.
      */
      SymbolParser(const std::string& symbol);

      //! Constructs a SymbolParser.
      /*!
        \param symbol The symbol to match.
      */
      SymbolParser(const char* symbol);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);

    private:
      std::string m_symbol;
  };

  //! Builds a SymbolParser.
  /*!
    \param symbol The symbol to match.
  */
  template<typename T>
  SymbolParser Symbol(const std::string& symbol) {
    return SymbolParser(symbol);
  }

  //! Builds a Parser that matches a symbol and returns a value.
  /*!
    \param symbol The symbol to match.
    \param value The value to return when the <i>symbol</i> is matched.
  */
  template<typename T>
  ConversionParser<SymbolParser, Details::SymbolConverter<T>> Symbol(
      const std::string& symbol, const T& value) {
    return Convert(SymbolParser(symbol), Details::SymbolConverter<T>(value));
  }

  inline SymbolParser::SymbolParser(const std::string& symbol)
      : m_symbol(symbol) {}

  inline SymbolParser::SymbolParser(const char* symbol)
      : m_symbol(symbol) {}

  template<typename ParserStreamType>
  bool SymbolParser::Read(ParserStreamType& source) {
    SubParserStream<ParserStreamType> context(source);
    for(char c : m_symbol) {
      if(!context.Read()) {
        return false;
      }
      if(context.GetChar() != c) {
        return false;
      }
    }
    context.Accept();
    return true;
  }
}
}

#endif
