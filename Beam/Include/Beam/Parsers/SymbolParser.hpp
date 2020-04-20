#ifndef BEAM_SYMBOLPARSER_HPP
#define BEAM_SYMBOLPARSER_HPP
#include <string>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam::Parsers {
namespace Details {
  template<typename T>
  struct SymbolConverter {
    T m_value;

    SymbolConverter(T value)
      : m_value(std::move(value)) {}

    T operator ()() const {
      return m_value;
    }
  };
}

  /*! \class Symbol
      \brief Matches a symbol.
   */
  class SymbolParser {
    public:
      using Result = NullType;

      //! Constructs a SymbolParser.
      /*!
        \param symbol The symbol to match.
      */
      SymbolParser(std::string symbol);

      //! Constructs a SymbolParser.
      /*!
        \param symbol The symbol to match.
      */
      SymbolParser(const char* symbol);

      template<typename Stream>
      bool Read(Stream& source) const;

    private:
      std::string m_symbol;
  };

  //! Builds a SymbolParser.
  /*!
    \param symbol The symbol to match.
  */
  inline auto Symbol(std::string symbol) {
    return SymbolParser(std::move(symbol));
  }

  //! Builds a Parser that matches a symbol and returns a value.
  /*!
    \param symbol The symbol to match.
    \param value The value to return when the <i>symbol</i> is matched.
  */
  template<typename T>
  auto Symbol(std::string symbol, T value) {
    return Convert(SymbolParser(std::move(symbol)),
      Details::SymbolConverter<T>(std::move(value)));
  }

  inline SymbolParser::SymbolParser(std::string symbol)
    : m_symbol(std::move(symbol)) {}

  inline SymbolParser::SymbolParser(const char* symbol)
    : m_symbol(symbol) {}

  template<typename Stream>
  bool SymbolParser::Read(Stream& source) const {
    auto context = SubParserStream<Stream>(source);
    for(auto c : m_symbol) {
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

#endif
