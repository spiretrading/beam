#ifndef BEAM_SYMBOL_PARSER_HPP
#define BEAM_SYMBOL_PARSER_HPP
#include <string>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {

  /** Matches a symbol. */
  class SymbolParser {
    public:
      using Result = void;

      /**
       * Constructs a SymbolParser.
       * @param symbol The symbol to match.
       */
      SymbolParser(std::string symbol) noexcept;

      /**
       * Constructs a SymbolParser.
       * @param symbol The symbol to match.
       */
      SymbolParser(const char* symbol);

      template<IsParserStream S>
      bool read(S& source) const;

    private:
      std::string m_symbol;
  };

  /**
   * Returns a SymbolParser.
   * @param symbol The symbol to match.
   */
  inline auto symbol(std::string symbol) {
    return SymbolParser(std::move(symbol));
  }

  inline SymbolParser::SymbolParser(std::string symbol) noexcept
    : m_symbol(std::move(symbol)) {}

  inline SymbolParser::SymbolParser(const char* symbol)
    : m_symbol(symbol) {}

  template<IsParserStream S>
  bool SymbolParser::read(S& source) const {
    auto context = SubParserStream<S>(source);
    for(auto c : m_symbol) {
      if(!context.read() || context.peek() != c) {
        return false;
      }
    }
    context.accept();
    return true;
  }
}

#endif
