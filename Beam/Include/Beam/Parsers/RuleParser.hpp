#ifndef BEAM_RULEPARSER_HPP
#define BEAM_RULEPARSER_HPP
#include <type_traits>
#include "Beam/Parsers/NoneParser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/VirtualParser.hpp"
#include "Beam/Parsers/VirtualParserStream.hpp"

namespace Beam::Parsers {

  /*! \class RuleParser
      \brief Used to represent any generic Parser.
      \tparam R The data type storing the parsed value.
   */
  template<typename R>
  class RuleParser {
    public:
      using Result = R;

      //! Constructs a RuleParser.
      RuleParser();

      //! Constructs a RuleParser.
      template<typename Parser>
      RuleParser(Parser parser);

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;

      //! Sets the parser to apply.
      template<typename Parser>
      void SetRule(Parser parser);

    protected:

      //! Constructs a RuleParser.
      template<typename Parser, typename = std::enable_if_t<
        !std::is_constructible_v<RuleParser, std::decay_t<Parser>>>>
      RuleParser(Parser parser);

    private:
      std::shared_ptr<std::unique_ptr<VirtualParser<Result>>> m_source;
  };

  template<typename R>
  RuleParser<R>::RuleParser()
    : RuleParser(NoneParser<Result>()) {}

  template<typename R>
  template<typename Parser>
  RuleParser<R>::RuleParser(Parser parser)
      : m_source(std::make_shared<std::unique_ptr<VirtualParser<Result>>>()) {
    SetRule(std::move(parser));
  }

  template<typename R>
  template<typename Stream>
  bool RuleParser<R>::Read(Stream& source, Result& value) const {
    return (*m_source)->Read(source, value);
  }

  template<typename R>
  template<typename Stream>
  bool RuleParser<R>::Read(Stream& source) const {
    return (*m_source)->Read(source);
  }

  template<typename R>
  template<typename Parser>
  void RuleParser<R>::SetRule(Parser parser) {
    using CastParser = decltype(Cast<Result>(parser));
    *m_source = std::make_unique<WrapperParser<CastParser>>(
      Cast<Result>(std::move(parser)));
  }

  template<typename R>
  template<typename Parser, typename>
  RuleParser<R>::RuleParser(Parser parser) {
    SetRule(std::move(parser));
  }
}

#endif
