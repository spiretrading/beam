#ifndef BEAM_DEFERRED_PARSER_HPP
#define BEAM_DEFERRED_PARSER_HPP
#include <type_traits>
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/NoneParser.hpp"

namespace Beam {

  /**
   * Parser used to defer initialization.
   * @tparam R The data type storing the parsed value.
   */
  template<typename R>
  class DeferredParser {
    public:
      using Result = R;

      /** Constructs a DeferredParser. */
      DeferredParser();

      /** Constructs a DeferredParser. */
      template<IsParser P,
        typename = disable_copy_constructor_t<DeferredParser, P>> requires
          std::convertible_to<parser_result_t<P>, R>
      DeferredParser(P parser);

      /** Sets the parser to apply. */
      template<IsParser P> requires std::convertible_to<parser_result_t<P>, R>
      void set(P parser);

      template<IsParserStream S>
      bool read(S& source, Result& value) const;
      template<IsParserStream S>
      bool read(S& source) const;

    private:
      std::shared_ptr<std::shared_ptr<Parser<Result>>> m_parser;
  };

  template<typename P>
  DeferredParser(P) -> DeferredParser<parser_result_t<P>>;

  template<typename R>
  DeferredParser<R>::DeferredParser()
    : DeferredParser(none_p<Result>) {}

  template<typename R>
  template<IsParser P, typename> requires
    std::convertible_to<parser_result_t<P>, R>
  DeferredParser<R>::DeferredParser(P parser)
    : m_parser(std::make_shared<std::shared_ptr<Parser<Result>>>(
        std::make_shared<Parser<Result>>(cast<Result>(std::move(parser))))) {}

  template<typename R>
  template<IsParserStream S>
  bool DeferredParser<R>::read(S& source, Result& value) const {
    return (*m_parser)->read(source, value);
  }

  template<typename R>
  template<IsParserStream S>
  bool DeferredParser<R>::read(S& source) const {
    return (*m_parser)->read(source);
  }

  template<typename R>
  template<IsParser P> requires std::convertible_to<parser_result_t<P>, R>
  void DeferredParser<R>::set(P parser) {
    *m_parser =
      std::make_shared<Parser<Result>>(cast<Result>(std::move(parser)));
  }
}

#endif
