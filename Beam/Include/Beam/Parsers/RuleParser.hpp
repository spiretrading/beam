#ifndef BEAM_RULE_PARSER_HPP
#define BEAM_RULE_PARSER_HPP
#include <type_traits>
#include "Beam/Parsers/NoneParser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/ParserBox.hpp"
#include "Beam/Parsers/ParserStreamBox.hpp"
#include "Beam/Utilities/TypeList.hpp"

namespace Beam::Parsers {

  /**
   * Used to represent any generic Parser.
   * @param <R> The data type storing the parsed value.
   */
  template<typename R>
  class RuleParser {
    public:
      using Result = R;

      /** Constructs a RuleParser. */
      RuleParser();

      /** Constructs a RuleParser. */
      template<typename Parser, typename =
        disable_copy_constructor_t<RuleParser, Parser>>
      RuleParser(Parser parser);

      /** Sets the parser to apply. */
      template<typename Parser>
      void SetRule(Parser parser);

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;

    private:
      std::shared_ptr<ParserBox<Result>> m_source;
  };

  template<typename R>
  RuleParser<R>::RuleParser()
    : RuleParser(NoneParser<Result>()) {}

  template<typename R>
  template<typename Parser, typename>
  RuleParser<R>::RuleParser(Parser parser)
      : m_source(std::make_shared<ParserBox<Result>>(NoneParser<Result>())) {
    SetRule(std::move(parser));
  }

  template<typename R>
  template<typename Stream>
  bool RuleParser<R>::Read(Stream& source, Result& value) const {
    return m_source->Read(source, value);
  }

  template<typename R>
  template<typename Stream>
  bool RuleParser<R>::Read(Stream& source) const {
    return m_source->Read(source);
  }

  template<typename R>
  template<typename Parser>
  void RuleParser<R>::SetRule(Parser parser) {
    using CastParser = decltype(Cast<Result>(parser));
    *m_source = Cast<Result>(std::move(parser));
  }
}

#endif
